#!/usr/bin/env python3
import chigger

window = chigger.Window(size=(300,300))
viewport = chigger.Viewport()
chigger.annotations.Logo(logo='chigger', position=(0.5, 0.5), width=0.67, halign='center', valign='center')
window.start()
