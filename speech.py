from datetime import datetime
from pathlib import Path
import speech_recognition as sr
import io
import tempfile
import wave
from faster_whisper import WhisperModel

model_size = "small.en"

model = WhisperModel(model_size, device="cpu", compute_type="int8")

def listen_for_command(recognizer, mic):
    try:
        with mic as source:

            audio = recognizer.listen(source,timeout =5, phrase_time_limit=5)

            with tempfile.NamedTemporaryFile(suffix=".wav", delete=False) as temp_wav:
                temp_wav.write(audio.get_wav_data())

                temp_wav_path = temp_wav.name
            
            segments, info = model.transcribe(temp_wav_path, beam_size=5)

            transcript_slice = []

            for segment in segments:
                segment_text = segment.text.strip()
                transcript_slice.append(segment_text)

            transcript = " ".join(transcript_slice).strip()
    
    except wav.error:
        print("Error Writing WAV File")
    
    return transcript