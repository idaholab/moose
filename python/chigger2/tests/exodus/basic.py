#!/usr/bin/env python
import chigger


window = chigger.Window()
observer = chigger.observers.MainWindowObserver(window)
viewport = chigger.Viewport(window)

reader = chigger.exodus.ExodusReader('../input/mug_blocks_out.e')
source = chigger.exodus.ExodusSource(viewport, reader, variable='diffused', edges=True, cmap='viridis')


window.start()
