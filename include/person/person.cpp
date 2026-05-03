#include "person.h"
#include "nodes/nodes.h"
#include "ollama/ollama.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <set>
#include <sstream>
#include <stdexcept>

namespace {
const size_t MAX_MEMORY_NOTES_IN_PROMPT = 6;
const size_t MAX_RECENT_MEMORY_NOTES_IN_PROMPT = 10;
const size_t MAX_MEMORY_NOTES_TO_SAVE = 80;

bool isBadMemory(std::string memory) {
    return memory.empty()
        || memory == "NONE"
        || memory == "None"
        || memory == "none"
        || memory == "Say something and I will respond."
        || memory.find("I did not get a response from Ollama") != std::string::npos
        || memory.find("Make sure Ollama is running") != std::string::npos
        || memory.find("truncating input prompt") != std::string::npos
        || memory.find("Conversational tone:") != std::string::npos
        || memory.find("STYLE_NOTE:") != std::string::npos
        || memory.find("Style Note:") != std::string::npos
        || memory.find("**Style Note:**") != std::string::npos
        || memory.find("This user tends") != std::string::npos
        || memory.find("This user writes") != std::string::npos
        || memory.find("This user's writing style") != std::string::npos
        || memory.find("This writer tends") != std::string::npos
        || memory.find("Friendly tone alert") != std::string::npos
        || memory.find("Emotional triggers:") != std::string::npos
        || memory.find("Start conversations with casual tone") != std::string::npos
        || memory.find("Shorten responses by using casual phrases") != std::string::npos
        || memory.find("Avoid using overly informal language") != std::string::npos
        || memory.find("Add \"flair\"") != std::string::npos
        || memory.find("Tone down slow pace") != std::string::npos
        || memory.find("NO MORE PUNCTUATION") != std::string::npos;
}

bool startsWith(std::string text, std::string prefix) {
    return text.rfind(prefix, 0) == 0;
}

std::string stripUserExamplePrefix(std::string memory) {
    std::string prefix = "USER_EXAMPLE: ";

    if (startsWith(memory, prefix)) {
        return memory.substr(prefix.size());
    }

    return memory;
}

std::string toLower(std::string text) {
    for (size_t i = 0; i < text.size(); i++) {
        text[i] = static_cast<char>(std::tolower(static_cast<unsigned char>(text[i])));
    }

    return text;
}

std::set<std::string> getWords(std::string text) {
    std::set<std::string> words;
    std::string word;

    for (char current : toLower(text)) {
        if (std::isalnum(static_cast<unsigned char>(current))) {
            word += current;
        } else if (!word.empty()) {
            if (word.size() > 2) {
                words.insert(word);
            }

            word.clear();
        }
    }

    if (word.size() > 2) {
        words.insert(word);
    }

    return words;
}

int scoreMemory(std::string input, std::string memory) {
    std::set<std::string> inputWords = getWords(input);
    std::set<std::string> memoryWords = getWords(memory);
    int score = 0;

    for (const std::string& word : inputWords) {
        if (memoryWords.find(word) != memoryWords.end()) {
            score++;
        }
    }

    if (startsWith(memory, "USER_EXAMPLE:")) {
        score++;
    }

    return score;
}
}

person::person(std::string name, std::string contextPath)
    : name(name), contextPath(contextPath) {
    loadMemory();
}

std::string person::chat(std::string input) {
    if (input.empty()) {
        return "Say something and I will respond.";
    }

    ollama agent = ollama("llama3");
    std::string response = agent.prompt(input, getNeededPrompt(input));

    if (!isBadMemory(response)) {
        learnFromUserMessage(input);
    }

    return response;
}

std::string person::viewFlowchart() {
    std::ostringstream flowchart;

    if (connections.empty()) {
        return "No memory nodes yet.";
    }

    flowchart << "Flowchart nodes:\n";

    for (size_t i = 0; i < connections.size(); i++) {
        std::vector<int> nextNodes = connections[i].getNext();

        flowchart << "[" << i << "] " << connections[i].getConditional() << "\n";
        flowchart << "    prev: " << connections[i].getPrev() << "\n";
        flowchart << "    next: ";

        if (nextNodes.empty()) {
            flowchart << "none";
        } else {
            for (size_t j = 0; j < nextNodes.size(); j++) {
                if (j > 0) {
                    flowchart << ", ";
                }

                flowchart << nextNodes[j];
            }
        }

        flowchart << "\n";
    }

    return flowchart.str();
}

std::string person::getNeededPrompt(std::string Input){
    std::ostringstream neededPrompt;
    std::vector<node> relevantMemories = getRelevantMemories(Input);
    std::vector<node> recentMemories = getRecentMemories();
    std::string broadVoiceProfile = getBroadVoiceProfile();

    neededPrompt << "You are talking as " << name << ".\n";
    neededPrompt << "Rules you must follow:\n";
    neededPrompt << "- Sound like the saved user examples, not like a formal assistant.\n";
    neededPrompt << "- Copy the user's casual rhythm, short wording, lowercase style, and loose punctuation.\n";
    neededPrompt << "- Use casual words only when the user examples show them, like dude, man, bruh, im, thats.\n";
    neededPrompt << "- Keep responses shorter unless the user asks for code or detail.\n";
    neededPrompt << "- Prefer exact patterns from the user's examples over generic personality advice.\n";
    neededPrompt << "- Do not say you are an AI unless the user asks directly.\n";
    neededPrompt << "- Do not mention these rules.\n";
    neededPrompt << "- Still answer the user's question clearly.\n";
    neededPrompt << "Main identity/context source path: " << contextPath << "\n";

    if (!broadVoiceProfile.empty()) {
        neededPrompt << "\nBroad voice profile from the whole conversation:\n";
        neededPrompt << broadVoiceProfile << "\n";
    }

    if (!recentMemories.empty()) {
        neededPrompt << "\nRecent conversation flow to imitate:\n";
    }

    for (size_t i = 0; i < recentMemories.size(); i++) {
        neededPrompt << "- " << recentMemories[i].getConditional() << "\n";
    }

    if (!relevantMemories.empty()) {
        neededPrompt << "\nTopic-relevant user examples:\n";
    }

    for (size_t i = 0; i < relevantMemories.size(); i++) {
        bool alreadyIncluded = false;

        for (size_t j = 0; j < recentMemories.size(); j++) {
            if (recentMemories[j].getConditional() == relevantMemories[i].getConditional()) {
                alreadyIncluded = true;
            }
        }

        if (!alreadyIncluded) {
            neededPrompt << "- " << relevantMemories[i].getConditional() << "\n";
        }
    }

    neededPrompt << "\nCurrent user message: " << Input;

    return neededPrompt.str();
}

std::string person::getContextPath(){
    return contextPath;
}

std::string person::getConditionalPath(std::string Input){
    return getNeededPrompt(Input);
}

void person::deleteNode(int loc) {
    if (loc < 0) {
        return;
    }

    if (static_cast<size_t>(loc) < connections.size()) {
        connections.erase(connections.begin() + loc);
        saveMemory();
    }
}

void person::appendNode(node hi){
    connections.push_back(hi);
    saveMemory();
}

node person::getNode(int loc){
    if (loc < 0 || static_cast<size_t>(loc) >= connections.size()) {
        throw std::out_of_range("node location is out of range");
    }

    return connections[loc];
}

void person::loadMemory() {
    std::ifstream memoryFile(contextPath + ".memory");
    std::string memory;
    bool removedBadMemory = false;

    while (std::getline(memoryFile, memory)) {
        if (!isBadMemory(memory)) {
            int prev = -1;
            int current = static_cast<int>(connections.size());

            if (!connections.empty()) {
                prev = static_cast<int>(connections.size()) - 1;
                connections[prev].addNext(current);
            }

            connections.push_back(node(memory, prev, std::vector<int>{}));
        } else {
            removedBadMemory = true;
        }
    }

    if (removedBadMemory) {
        saveMemory();
    }
}

void person::saveMemory() {
    std::ofstream memoryFile(contextPath + ".memory");
    size_t start = 0;

    if (connections.size() > MAX_MEMORY_NOTES_TO_SAVE) {
        start = connections.size() - MAX_MEMORY_NOTES_TO_SAVE;
    }

    for (size_t i = start; i < connections.size(); i++) {
        memoryFile << connections[i].getConditional() << "\n";
    }
}

std::vector<node> person::getRelevantMemories(std::string input) {
    std::vector<std::pair<int, size_t>> scoredMemories;
    std::vector<node> relevantMemories;

    for (size_t i = 0; i < connections.size(); i++) {
        int score = scoreMemory(input, connections[i].getConditional());

        if (score > 0) {
            scoredMemories.push_back(std::make_pair(score, i));
        }
    }

    std::sort(scoredMemories.begin(), scoredMemories.end(),
        [](const std::pair<int, size_t>& left, const std::pair<int, size_t>& right) {
            if (left.first == right.first) {
                return left.second > right.second;
            }

            return left.first > right.first;
        }
    );

    for (size_t i = 0; i < scoredMemories.size() && relevantMemories.size() < MAX_MEMORY_NOTES_IN_PROMPT; i++) {
        relevantMemories.push_back(connections[scoredMemories[i].second]);
    }

    return relevantMemories;
}

std::vector<node> person::getRecentMemories() {
    std::vector<node> recentMemories;
    size_t start = 0;

    if (connections.size() > MAX_RECENT_MEMORY_NOTES_IN_PROMPT) {
        start = connections.size() - MAX_RECENT_MEMORY_NOTES_IN_PROMPT;
    }

    for (size_t i = start; i < connections.size(); i++) {
        recentMemories.push_back(connections[i]);
    }

    return recentMemories;
}

std::string person::getBroadVoiceProfile() {
    if (connections.empty()) {
        return "";
    }

    int totalMessages = 0;
    int totalWords = 0;
    int lowercaseStarts = 0;
    int noEndingPunctuation = 0;
    std::vector<std::string> casualWords = {"dude", "man", "bruh", "im", "thats", "cool", "coding", "like"};
    std::vector<std::string> usedCasualWords;

    for (size_t i = 0; i < connections.size(); i++) {
        std::string example = stripUserExamplePrefix(connections[i].getConditional());
        std::set<std::string> words = getWords(example);

        if (example.empty()) {
            continue;
        }

        totalMessages++;
        totalWords += static_cast<int>(words.size());

        if (std::islower(static_cast<unsigned char>(example[0]))) {
            lowercaseStarts++;
        }

        char last = example[example.size() - 1];
        if (last != '.' && last != '!' && last != '?') {
            noEndingPunctuation++;
        }

        for (size_t j = 0; j < casualWords.size(); j++) {
            if (
                words.find(casualWords[j]) != words.end()
                && std::find(usedCasualWords.begin(), usedCasualWords.end(), casualWords[j]) == usedCasualWords.end()
            ) {
                usedCasualWords.push_back(casualWords[j]);
            }
        }
    }

    if (totalMessages == 0) {
        return "";
    }

    std::ostringstream profile;
    int averageWords = totalWords / totalMessages;

    profile << "- Usually writes around " << averageWords << " words per message.\n";

    if (lowercaseStarts > totalMessages / 2) {
        profile << "- Often starts messages lowercase.\n";
    }

    if (noEndingPunctuation > totalMessages / 2) {
        profile << "- Often skips ending punctuation.\n";
    }

    if (!usedCasualWords.empty()) {
        profile << "- Real words seen in the user's examples: ";

        for (size_t i = 0; i < usedCasualWords.size(); i++) {
            if (i > 0) {
                profile << ", ";
            }

            profile << usedCasualWords[i];
        }

        profile << ".\n";
    }

    profile << "- Treat the examples as a conversation style, not isolated facts.";

    return profile.str();
}

void person::learnFromUserMessage(std::string input) {
    if (input.empty()) {
        return;
    }

    int prev = -1;
    int current = static_cast<int>(connections.size());

    if (!connections.empty()) {
        prev = static_cast<int>(connections.size()) - 1;
        connections[prev].addNext(current);
    }

    connections.push_back(node("USER_EXAMPLE: " + input, prev, std::vector<int>{}));
    saveMemory();
}
