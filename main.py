import os
import time
import winsound
import speech_recognition as sr
from dotenv import load_dotenv
from led_client import send_brightness_command
from led_client import send_triad
from speech import listen_for_command
from wakeword import detect_wakeword
from wakeword import reset_wakeword
from commands import parse_command


load_dotenv()
IP = os.getenv("ESP_IP")

is_active_mode = False
session_deadline = 0
SESSION_TIMEOUT = 15



recognizer = sr.Recognizer()
mic = sr.Microphone()

print("Calibrating microphone...")
with mic as source:
    recognizer.adjust_for_ambient_noise(source, duration=1)
print("System ready.")


# MAIN LOOP

while True:
    if is_active_mode:
        if time.time() > session_deadline:

            winsound.PlaySound(
                "wakeword/shutdown.wav",
                winsound.SND_FILENAME | winsound.SND_ASYNC
            )

            
            reset_wakeword()

            is_active_mode = False
            print("Session ended.")
            continue

        print("Listening...")
        transcript = listen_for_command(recognizer, mic)


        if transcript:
            print("You said:", transcript)

            actions = parse_command(transcript)

            data_sent = False

            for action in actions: 
                if action["type"] == "preset":
                    response_command = send_triad(action["name"], IP)

                    if response_command:
                        data_sent = True
                    print(f"Color command sent : {action['name']}")

                if action["type"] == "brightness":
                    response_brightness = send_brightness_command(action["value"], IP)

                    if response_brightness:
                        data_sent = True
                    print(f"Brightness command sent : {action['value']}")

                

            if data_sent:
                session_deadline = time.time() + SESSION_TIMEOUT
                print("Session renewed.")


    else:

        if detect_wakeword():

            print("Wakeword detected.")

            winsound.PlaySound(
                "wakeword/alexo_active.wav",
                winsound.SND_FILENAME | winsound.SND_ASYNC
            )

            is_active_mode = True
            session_deadline = time.time() + SESSION_TIMEOUT
            print("Listening...")