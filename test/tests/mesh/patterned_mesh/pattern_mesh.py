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
python pattern_mesh.py
"""
import vtk
import chigger

name = 'patterned_mesh_in'

reader = chigger.exodus.ExodusReader('{}.e'.format(name))
result = chigger.exodus.ExodusResult(reader, camera=None, color=[0.15,0,0.8], edges=True, edge_color=[0,0,0])

extents = chigger.misc.VolumeAxes(result)
extents.setOptions('xaxis', color=[0, 0, 0], minor_ticks=True)
extents.setOptions('yaxis', color=[0, 0, 0], minor_ticks=True)
extents.setOptions('zaxis', color=[0, 0, 0], minor_ticks=True)

window = chigger.RenderWindow(result, extents, antialiasing=5)
window.setOptions(size=[600,600], background=[0.7058823529411765, 0.7058823529411765, 0.7058823529411765], background2=[0.43529411764705883, 0.43529411764705883, 0.43529411764705883], gradient_background=True)
window.update()
window.resetCamera()
window.write('{}.png'.format(name))
