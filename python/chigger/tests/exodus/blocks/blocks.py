#!/usr/bin/env python
import vtk
import chigger

camera = vtk.vtkCamera()
camera.SetViewUp(-0.01297019406812408, 0.87867984226827, 0.4772352762079132)
camera.SetPosition(10.331000991784688, -5.473421359648077, 10.483371124667542)
camera.SetFocalPoint(0.16947273724857123, 0.07124492441302266, -0.0015694043706061533)

reader = chigger.exodus.ExodusReader('../../input/mug_blocks_out.e')
mug = chigger.exodus.ExodusResult(reader, block=['76'], variable='convected', cmap='viridis', camera=camera)
window = chigger.RenderWindow(mug, size=[300,300], test=True)
window.write('blocks.png')
window.start()
