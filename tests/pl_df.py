# coding: utf-8
import struct
import pandas as pd
import matplotlib.pyplot as plt
df = pd.read_csv('toto.csv', comment="#")

def to_float(s):
    return [ struct.unpack('>f', bytes.fromhex(d))[0] for d in s]

# we get only the two first columns
df = df.apply(to_float)
df.plot()
plt.show()
