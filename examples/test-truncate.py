#!/usr/bin/env python
import audio

w = audio.BaseWave()
w.open('./demo.wav')
print w

waves = audio.BaseWaveVec()
w.smart_truncate(5*60, waves)
for slice in waves:
    slice.write()
