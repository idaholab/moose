#!/usr/bin/env python
import os
import chigger

names = ['default', 'coolwarm', 'grayscale', 'rainbow', 'jet', 'shock', 'viridis', 'inferno', 'magma', 'plasma', 'Blues', 'Greens', 'Reds']
for name in names:
    fname = os.path.realpath(os.path.join('..', 'icons', 'colormaps', name + ".png"))
    colorbar = chigger.misc.ColorBar(cmap=name, location='top', width=1.02, length=1.01, colorbar_origin=[0,0])
    window = chigger.RenderWindow(colorbar, size=[400, 200])
    window.write(fname)
