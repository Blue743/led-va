
import re

commands = {
    "brisa" : {
    "colors" : [
        (255, 132, 0),
        (137, 255, 0),
        (0, 255, 92),
        (0, 255, 242),
    ],
    "mode" : "triad",
    "brightness" : 80
    },

    "intercalado" : {
    "colors" : [
            (255, 132, 0),
            (137, 255, 0),
    ],
    "mode" : "duo",
    "brightness" : 80
    },

    "aleatório" : {
    "colors" : [
        (255, 132, 0),
        (137, 255, 0),
        (0, 255, 174),
        (0, 162, 255),
        (74, 0, 255),
        (206, 0, 255),
        (255, 0, 119),
        (255, 40, 40),
    ],
    "mode" : "random",
    "brightness" : 80
    },

    "constante" : {
    "colors" : [
        (255, 132, 0),
        (137, 255, 0),
        (0, 255, 174),
    ],
    "mode" : "one",
    "brightness" : 80
    },

    "inteira" : {
    "colors" : [
        (255, 132, 0),
        (137, 255, 0),
        (0, 255, 174),
    ],
    "mode" : "all",
    "brightness" : 80
    },

    "estrela" : {
    "colors" : [
        (75, 89, 252),
    ],
    "mode" : "comet",
    "brightness" : 255
    }
}


def parse_command(transcript):
    """Convert a transcript into actions the application can execute."""
    text = transcript.strip().casefold()

    if re.search(r"\b(parar|encerrar)\b", text):
        return [{"type": "end_session"}]

    actions = []

    for preset_name in commands:
        if re.search(rf"\b{re.escape(preset_name)}\b", text):
            actions.append({"type": "preset", "name": preset_name})
            break

    brightness_match = re.search(r"\bbrilho(?:\s+de)?\s+(\d{1,3})\b", text)
    if brightness_match:
        brightness = int(brightness_match.group(1))
        if 0 <= brightness <= 255:
            actions.append({"type": "brightness", "value": brightness})
        else:
            print("Brightness must be between 0 and 255.")

    return actions
