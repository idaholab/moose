#!/usr/bin/env python3

from moosetools import chigger

window = chigger.Window(observer=True)
viewport = chigger.Viewport()
rect = chigger.geometric.Rectangle(xmin=0.2, xmax=0.8, ymin=0.2, ymax=0.8, color=chigger.utils.Color(0.3,0.4,0.5))
window.start()
