import speech_recognition as sr
import os
import tempfile
from faster_whisper import WhisperModel

model_size = "small.en"

model = WhisperModel(model_size, device="cpu", compute_type="int8")

def listen_for_command(recognizer, mic):
    temp_wav_path = None

    try:
        with mic as source:
            audio = recognizer.listen(source, timeout=5, phrase_time_limit=5)

        with tempfile.NamedTemporaryFile(suffix=".wav", delete=False) as temp_wav:
            temp_wav.write(audio.get_wav_data())
            temp_wav_path = temp_wav.name

        segments, _ = model.transcribe(
            temp_wav_path,
            vad_filter=True,
            vad_parameters={
                "min_silence_duration_ms" : 500,
                "threshold" : 0.5,
            },
            condition_on_previous_text=False,
            no_speech_threshold = 0.6,
            log_prob_threshold = -1.0,
            beam_size=3,
            language="pt",
        )
        transcript = " ".join(
            segment.text.strip()
            for segment in segments
            if segment.text.strip()
        )
        return transcript or None

    except sr.WaitTimeoutError:
        print("No speech detected before timeout.")
        return None
    except Exception as exc:
        print(f"Speech recognition failed: {exc}")
        return None
    finally:
        if temp_wav_path and os.path.exists(temp_wav_path):
            os.remove(temp_wav_path)


