#!/usr/bin/env python3

import vtk
import chigger

camera = vtk.vtkCamera()
camera.SetViewUp(0.0000000000, 1.0000000000, 0.0000000000)
camera.SetPosition(0.4940404045, -0.1814478046, 1.1120756524)
camera.SetFocalPoint(0.4940404045, -0.1814478046, 0.0000000000)

reader = chigger.exodus.ExodusReader('2d_main_out.e')
result = chigger.exodus.ExodusResult(reader, variable='T', cmap='viridis', camera=camera, range=[263.15, 273.15])
cbar = chigger.exodus.ExodusColorBar(result, location='bottom')
cbar.setOptions('primary', title='Temperature [K]', font_size=32)
text = chigger.annotations.TextAnnotation(font_size=32, position=[0.5, 0.3], justification='center')
window = chigger.RenderWindow(result, cbar, text, size=[1920,1080], test=True)

for i, t in enumerate(reader.getTimes()):
    reader.update(timestep=i)
    text.setOptions(text='Time {:05d} sec.'.format(int(t)))
    window.write('2d_main_{:03d}.png'.format(i))
window.start()

# Leave this commented out, don't want to worry about setting up ffmpeg on test machines
#chigger.utils.img2mov('2d_main_*.png', '2d_main.mp4', duration=20, overwrite=True)
