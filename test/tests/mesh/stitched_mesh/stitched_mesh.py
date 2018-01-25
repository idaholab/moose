#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

"""
Creates images for StitchedMesh documentation.
"""
import vtk
import chigger

name = 'stitched_mesh_out'
camera = vtk.vtkCamera()
size = [600,600]
if name == 'left':
    camera.SetViewUp(0.0000, 1.0000, 0.0000)
    camera.SetPosition(0.4587, 0.4610, 2.5937)
    camera.SetFocalPoint(0.4587, 0.4610, 0.0000)
    out = 'stitched_mesh_left.png'
elif name == 'center':
    camera.SetViewUp(0.0000, 1.0000, 0.0000)
    camera.SetPosition(1.4587, 0.4610, 2.5937)
    camera.SetFocalPoint(1.4587, 0.4610, 0.0000)
    out = 'stitched_mesh_center.png'
elif name == 'right':
    camera.SetViewUp(0.0000, 1.0000, 0.0000)
    camera.SetPosition(2.4587, 0.4610, 2.5937)
    camera.SetFocalPoint(2.4587, 0.4610, 0.0000)
    out = 'stitched_mesh_right.png'
elif name == 'stitched_mesh_out':
    camera.SetViewUp(0.0000, 1.0000, 0.0000)
    camera.SetPosition(1.4587, 0.4610, 3.1575)
    camera.SetFocalPoint(1.4587, 0.4610, -0.0000)
    size = [900, 400]
    out = 'stitched_mesh_out.png'

reader = chigger.exodus.ExodusReader('{}.e'.format(name))
result = chigger.exodus.ExodusResult(reader, camera=camera, edges=True, edge_color=[0,0,0], color=[1,1,1])

extents = chigger.misc.VolumeAxes(result)
extents.setOptions('xaxis', color=[0, 0, 0], minor_ticks=True)
extents.setOptions('yaxis', color=[0, 0, 0], minor_ticks=True)

window = chigger.RenderWindow(result, extents, antialiasing=100)
window.setOptions(size=size, style='interactive2D', background=[0.7058823529411765, 0.7058823529411765, 0.7058823529411765], background2=[0.43529411764705883, 0.43529411764705883, 0.43529411764705883], gradient_background=True)
window.resetCamera()
window.update()
window.write(out)
#window.start()
#print '\n'.join(chigger.utils.print_camera(result.getVTKRenderer().GetActiveCamera()))
