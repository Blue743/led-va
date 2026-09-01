import requests
from commands import commands


def send_brightness_command(brightness, ip_address):
    try:
        response = requests.get(
            f"http://{ip_address}/bright?value={brightness}",
             timeout=5
             )
             
        response.raise_for_status()
        return True
    except requests.RequestException as exc:
        print(f"Error connecting to ESP: {exc}")
        return False


def send_triad(preset_name, ip_address):
    preset_config = commands[preset_name]
    print(f"Command {preset_name} found.")

    try:
        response = requests.post(
            f"http://{ip_address}/apply",
            json=preset_config,
            timeout=5,
        )
        response.raise_for_status()
        return True
    except requests.RequestException as exc:
        print(f"Cannot reach ESP: {exc}")
        return False

