#pylint: disable=missing-docstring
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from __future__ import print_function
import vtk
from .. import utils

class KeyPressInteractorStyle(vtk.vtkInteractorStyleMultiTouchCamera):
    """
    An interactor style for capturing key press events in VTK window.
    """
    def __init__(self, parent=None, **kwargs):
        self.AddObserver("KeyPressEvent", self.keyPress)
        super(KeyPressInteractorStyle, self).__init__(parent, **kwargs)

    def keyPress(self, obj, event): #pylint: disable=unused-argument
        """
        Executes when a key is pressed.

        Inputs:
            obj, event: Required by VTK.
        """
        key = obj.GetInteractor().GetKeySym()
        if key == 'c':
            print('\n'.join(utils.print_camera(self.GetCurrentRenderer().GetActiveCamera())))
