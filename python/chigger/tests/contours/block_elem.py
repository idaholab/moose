#!/usr/bin/env python
import vtk
import chigger

camera = vtk.vtkCamera()
camera.SetViewUp(0.1829, 0.7889, 0.5867)
camera.SetPosition(-9.9663, -4.0748, 7.8279)
camera.SetFocalPoint(0.0000, 0.0000, -0.7582)

reader = chigger.exodus.ExodusReader('../input/mug_blocks_out.e')

contour = chigger.filters.ContourFilter()
result = chigger.exodus.ExodusResult(reader, variable='aux_elem', camera=camera, block=['76'], filters=[contour])

window = chigger.RenderWindow(result, size=[300,300], test=True)
window.write('block_elem.png')
window.start()
