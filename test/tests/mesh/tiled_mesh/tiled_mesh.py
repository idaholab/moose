#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

"""
Creates images for TiledMesh documentation.
python tiled_mesh.py
"""
import vtk
import chigger

name = 'cube.e'
name = 'tiled_mesh_test_in.e'
if name == 'cube.e':
    out = 'tiled_mesh_input.png'
else:
    out = 'tiled_mesh_output.png'

camera = vtk.vtkCamera()
camera.SetViewUp(-0.2136, 0.8540, -0.4745)
camera.SetPosition(23.7499, 21.0876, 27.5908)
camera.SetFocalPoint(4.9242, 4.2034, 5.6789)

reader = chigger.exodus.ExodusReader(name)
result = chigger.exodus.ExodusResult(reader, camera=camera, color=[0,0.85,0.85], edges=True, edge_color=[0,0,0])

extents = chigger.misc.VolumeAxes(result)
extents.setOptions('xaxis', color=[0, 0, 0], minor_ticks=True)
extents.setOptions('yaxis', color=[0, 0, 0], minor_ticks=True)
extents.setOptions('zaxis', color=[0, 0, 0], minor_ticks=True)

window = chigger.RenderWindow(result, extents, antialiasing=5)
window.setOptions(size=[600,600], background=[0.7058823529411765, 0.7058823529411765, 0.7058823529411765], background2=[0.43529411764705883, 0.43529411764705883, 0.43529411764705883], gradient_background=True)
window.update()
window.resetCamera()
window.write(out)
#window.start()
#print '\n'.join(chigger.utils.print_camera(result.getVTKRenderer().GetActiveCamera()))
