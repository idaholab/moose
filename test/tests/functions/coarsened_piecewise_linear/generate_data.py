#!/usr/bin/env python3

import numpy as np

x = np.linspace(0, np.pi*10, 10000)
y = np.cos(x) * np.exp(-x/10.0)

np.savetxt("input.csv", np.transpose([x,y]), delimiter=",")
