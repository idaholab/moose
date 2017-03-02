#!/usr/bin/env python
import chigger
colorbar = chigger.misc.ColorBar(cmap='viridis')
colorbar.setOptions('primary', lim=[5,10])
colorbar.setOptions('secondary', lim=[100,500], visible=True)
window = chigger.RenderWindow(colorbar, size=[600,400], test=True)
window.write('colorbar.png')
window.start()
