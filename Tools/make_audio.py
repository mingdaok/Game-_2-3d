"""Original synthesized sounds; no external audio samples are used."""
from pathlib import Path
import math
import random
import struct
import wave

dest = Path(__file__).parent / "GeneratedAudio"
dest.mkdir(exist_ok=True)
rate = 22050
rng = random.Random(613)

def write(name, seconds, generator):
    data = [generator(i / rate, seconds) for i in range(int(seconds * rate))]
    peak = max(max(abs(v) for v in data), 0.01)
    with wave.open(str(dest / (name + ".wav")), "wb") as out:
        out.setparams((1, 2, rate, 0, "NONE", "not compressed"))
        out.writeframes(b"".join(struct.pack("<h", int(v / peak * 22000)) for v in data))

write("S_Fold", .55, lambda t, d: math.sin(math.pi*t/d)**2 * (.4*rng.uniform(-1,1) + .55*math.sin(2*math.pi*(150*t+650*t*t))))
write("S_Fire", .75, lambda t, d: min(t*40,1)*(1-t/d)**2 * (.7*rng.uniform(-1,1)+.15*math.sin(2*math.pi*75*t)))
write("S_Restore", 1.7, lambda t, d: min(t*30,1)*math.exp(-3*t) * sum(math.sin(2*math.pi*f*t)*a for f,a in [(440,.45),(554.37,.3),(659.25,.3),(880,.15)]))
print("Created three original WAV files in", dest)
