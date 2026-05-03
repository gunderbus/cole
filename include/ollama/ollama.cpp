#include "ollama.h"

#include <array>
#include <cstdio>
#include <sstream>

namespace {
std::string escapeJson(std::string text) {
    std::string escaped;

    for (char current : text) {
        if (current == '\\') {
            escaped += "\\\\";
        } else if (current == '"') {
            escaped += "\\\"";
        } else if (current == '\n') {
            escaped += "\\n";
        } else {
            escaped += current;
        }
    }

    return escaped;
}

std::string escapeShell(std::string text) {
    std::string escaped = "'";

    for (char current : text) {
        if (current == '\'') {
            escaped += "'\\''";
        } else {
            escaped += current;
        }
    }

    escaped += "'";
    return escaped;
}

std::string unescapeJsonString(std::string text) {
    std::string unescaped;

    for (size_t i = 0; i < text.size(); i++) {
        if (text[i] != '\\' || i + 1 >= text.size()) {
            unescaped += text[i];
            continue;
        }

        i++;
        if (text[i] == 'n') {
            unescaped += '\n';
        } else if (text[i] == 't') {
            unescaped += '\t';
        } else {
            unescaped += text[i];
        }
    }

    return unescaped;
}

std::string extractResponse(std::string json) {
    std::string key = "\"response\":\"";
    std::string response;
    size_t searchStart = 0;

    while (true) {
        size_t start = json.find(key, searchStart);
        if (start == std::string::npos) {
            break;
        }

        start += key.size();
        std::string chunk;
        bool escaped = false;

        for (size_t i = start; i < json.size(); i++) {
            if (!escaped && json[i] == '"') {
                searchStart = i + 1;
                break;
            }

            if (!escaped && json[i] == '\\') {
                escaped = true;
            } else {
                escaped = false;
            }

            chunk += json[i];
        }

        response += unescapeJsonString(chunk);
    }

    return response;
}
}

ollama::ollama(std::string modelName) : model(modelName) {
}

// Method to "ask" the AI
std::string ollama::prompt(std::string userInput, std::string context) {
    std::string fullPrompt = context + "\n\nUser: " + userInput + "\nAssistant:";
    std::string body = "{\"model\":\"" + escapeJson(model) + "\",\"prompt\":\"" + escapeJson(fullPrompt) + "\"}";
    std::string command = "curl -s " + endpoint + " -d " + escapeShell(body);
    std::array<char, 256> buffer;
    std::string output;
    FILE* pipe = popen(command.c_str(), "r");

    if (pipe == nullptr) {
        return "I could not connect to Ollama.";
    }

    while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
        output += buffer.data();
    }

    pclose(pipe);

    std::string response = extractResponse(output);
    if (response.empty()) {
        return "I did not get a response from Ollama. Make sure Ollama is running with the " + model + " model.";
    }

    return response;
}
    
bool ollama::getIsTrue(std::string isTrue) {
    std::string answer = prompt(
        "Answer only yes or no: " + isTrue,
        "You judge whether a condition is true."
    );

    return answer.find("yes") != std::string::npos || answer.find("Yes") != std::string::npos;
}
