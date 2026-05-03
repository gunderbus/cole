import os
import subprocess
from pathlib import Path

import discord


ROOT = Path(__file__).resolve().parent
COLE_BINARY = ROOT / "cole"


def ask_cole_read_only(message: str) -> str:
    if not message.strip():
        return "say something dude"

    result = subprocess.run(
        [str(COLE_BINARY), "--chat-readonly", message],
        cwd=ROOT,
        capture_output=True,
        text=True,
        timeout=45,
    )

    if result.returncode != 0:
        return "dude something broke while i was thinking"

    response = result.stdout.strip()
    if not response:
        return "i got nothing back from ollama"

    return response


def split_discord_message(message: str) -> list[str]:
    if len(message) <= 1900:
        return [message]

    chunks = []
    start = 0

    while start < len(message):
        chunks.append(message[start:start + 1900])
        start += 1900

    return chunks


intents = discord.Intents.default()
intents.message_content = True
client = discord.Client(intents=intents)


@client.event
async def on_ready():
    print(f"cole discord bot logged in as {client.user}")


@client.event
async def on_message(message):
    if message.author.bot:
        return

    is_dm = isinstance(message.channel, discord.DMChannel)
    was_mentioned = client.user in message.mentions

    if not is_dm and not was_mentioned:
        return

    content = message.content

    if was_mentioned:
        content = content.replace(f"<@{client.user.id}>", "")
        content = content.replace(f"<@!{client.user.id}>", "")

    async with message.channel.typing():
        response = ask_cole_read_only(content.strip())

    for chunk in split_discord_message(response):
        await message.reply(chunk, mention_author=False)


token = os.getenv("DISCORD_BOT_TOKEN")

if not token:
    raise RuntimeError("Set DISCORD_BOT_TOKEN before running the Discord bot.")

client.run(token)
