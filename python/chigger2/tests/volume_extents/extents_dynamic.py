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

class Test(chigger.observers.TestObserver):

    def onTimer(self, *args):

        extents = chigger.misc.VolumeAxes(self._window._results[1])
        self._window.append(extents)
        self._window.write('extents_on.png')
        self._window.remove(extents)
        self._window.write('extents_off.png')
        self._window.append(extents)
        self._window.write('extents_on.png')
        self._window.remove(extents)
        self._window.write('extents_off.png')


camera = vtk.vtkCamera()
camera.SetViewUp(-0.0673, 0.8897, 0.4516)
camera.SetPosition(-5.3701, -6.9590, 13.0350)
camera.SetFocalPoint(0.0000, 0.0000, 0.1250)

reader = chigger.exodus.ExodusReader('../input/mug_blocks_out.e')
mug = chigger.exodus.ExodusResult(reader, variable='diffused', camera=camera, cmap='viridis')
window = chigger.RenderWindow(mug, size=(300,300), observers=[Test(terminate=True)])

window.start()
