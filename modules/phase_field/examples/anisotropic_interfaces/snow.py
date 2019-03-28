#!/usr/bin/env python2
import vtk
import chigger

camera = vtk.vtkCamera()
camera.SetViewUp(0.0000000000, 1.0000000000, 0.0000000000)
camera.SetPosition(4.5000000000, 4.5000000000, 11.4706967835)
camera.SetFocalPoint(4.5000000000, 4.5000000000, 0.0000000000)

reader = chigger.exodus.ExodusReader('snow_out.e')
result = chigger.exodus.ExodusResult(reader, variable='w', cmap='viridis', edges=True,
                                     camera=camera, edge_color=[0.25]*3)
window = chigger.RenderWindow(result, size=[600,600])

for i, t in enumerate(reader.getTimes()):
    reader.setOptions(timestep=i)
    window.write('snow_{:04d}.png'.format(i))
window.start()

chigger.utils.img2mov('snow_*.png', 'snow.mp4', duration=20)
