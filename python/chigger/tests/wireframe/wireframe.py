#!/usr/bin/env python
import vtk
import chigger

camera = vtk.vtkCamera()
camera.SetViewUp(-0.0112, -0.0149, 0.9998)
camera.SetPosition(-12.5229, -9.3291, -0.1552)
camera.SetFocalPoint(0.0000, 0.0000, 0.1250)

reader = chigger.exodus.ExodusReader('../input/mug_blocks_out.e')
mug = chigger.exodus.ExodusResult(reader, variable='diffused', representation='wireframe', camera=camera, colorbar={'visible':False}, cmap='viridis')
window = chigger.RenderWindow(mug, size=[300,300], test=True)
window.write('wireframe.png')
window.start()
