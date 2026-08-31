
import openwakeword
from openwakeword.model import Model
import pyaudio
import numpy as np

#definições constantes usadas na lógica de wakeword
WAKEWORD_MODEL_PATH = "models/luna.onnx"
WAKEWORD_LABEL = "luna"
WAKEWORD_THRESHOLD = 0.2
CHUNKSIZE = 1280
RATE = 16000

#definição do modelo.
model = Model(
    wakeword_models=[WAKEWORD_MODEL_PATH],
    inference_framework="onnx",
)


pa = pyaudio.PyAudio()

#aqui o pyAudio abre a leitura de audio e retorna o audio em Bytes
audio_stream = pa.open(
    rate= RATE,
    channels=1,
    format=pyaudio.paInt16,
    input=True,
    frames_per_buffer=CHUNKSIZE
)

#função para ler e converter o audio em bytes em um array numérico com o numpy
def detect_wakeword():
    data_bytes = audio_stream.read(CHUNKSIZE, exception_on_overflow=False)
    numeric_array = np.frombuffer(data_bytes, dtype=np.int16)
    prediction = model.predict(numeric_array)
    score = prediction[WAKEWORD_LABEL]
    return score > WAKEWORD_THRESHOLD


#função para resetar o buffer do pyAudio e previnir retorno false apos fim de sessão
def reset_wakeword():
    model.reset()
