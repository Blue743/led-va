
import re

commands = {
    "gelado" : {
    "colors" : [
        (0, 105, 255),
        (225, 235, 250)
    ],
    "mode" : "cold",
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
        (163, 122, 180),
        (212, 134, 184),
        (241, 177, 193),
        (171, 188, 214),
        (180, 216, 232)
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
    
    "cano" : {
    "colors" : [
        (0, 52, 255),
        (255, 255, 255),
        (255, 0, 0),
    ],
    "mode" : "pole",
    "brightness" : 150
    },


    "estrela" : {
    "colors" : [
        (138, 38, 245),
    ],
    "mode" : "comet",
    "brightness" : 255
    },

    "separado": {
    "colors": [
        (110, 20, 255),
        (185, 50, 255),
        (255, 45, 170),
        (255, 75, 70),
        (255, 125, 20),
        (255, 195, 30),
        (70, 150, 255),
        (35, 60, 255),
    ],
    "mode": "group",
    "brightness": 255
    }, 

    "cyberpunk": {
    "colors": [
    (255, 0, 150),
    (118, 61, 255),
    (226, 54, 68),
    (180, 100, 255),
    (0, 255, 210),
    (80, 20, 255),
    ],
    "mode": "cyberpunk",
    "brightness": 120
    },

    "cherry": {
    "colors": [
    (96, 136, 255),
    (255, 65, 114),
    (255, 35, 160),
    (226, 54, 68),
    (190, 78, 255),
    (62, 169, 176)
    ],
    "mode": "sakura",
    "brightness": 120
    },

    "estrelado": {
    "colors": [
    (67, 43, 252),
    (255, 255, 0),

    ],
    "mode": "galaxy",
    "brightness": 200
    }, 

    "múltiplo": {
    "colors": [
        (35, 90, 255),
        (255, 45, 140),
        (130, 45, 255),
        (189, 49, 49),
        (224, 91, 20),
        (173, 240, 202)
    ],
    "mode": "cycle",
    "brightness": 200
    },

        "desligar": {
        "colors": [
            (0,0,0)
        ],
        "mode": "off",
        "brightness": 0
        }
}

"ty soshi"
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
