#!/usr/bin/env python3
#pylint: disable=missing-docstring
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import vtk
import chigger

camera = vtk.vtkCamera()
camera.SetViewUp(-0.01297019406812408, 0.87867984226827, 0.4772352762079132)
camera.SetPosition(10.331000991784688, -5.473421359648077, 10.483371124667542)
camera.SetFocalPoint(0.16947273724857123, 0.07124492441302266, -0.0015694043706061533)

reader = chigger.exodus.ExodusReader('../../input/mug_blocks_out.e', boundary=['bottom', 'top'])
mug = chigger.exodus.ExodusResult(reader, block=None, boundary=['1'], variable='convected', cmap='coolwarm', camera=camera)
window = chigger.RenderWindow(mug, size=[300,300], test=True)
window.write('boundary_numeric.png')
#for key, value in reader.getBlockInformation().iteritems():
#    print key, value
window.start()
