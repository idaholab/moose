#!/usr/bin/env python3
from moosetools import chigger

window = chigger.Window(size=(300,300))
viewport = chigger.Viewport()
text = chigger.annotations.Text(text='chigger', halign='center', position=(0.5, 0.5))
text.setParams('frame', on=True, width=10)
window.start()
