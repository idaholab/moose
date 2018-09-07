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
from SingleShotObserver import SingleShotObserver
class TestObserver(SingleShotObserver):
    """
    Tool for simulating key strokes and mouse movements.
    """

    def pressKey(self, key, shift=False):
        """
        Simulate a key press.

        Inputs:
            key[str]: The key symbol (e.g. "k").
            shift[bool]: Flag for holding the shift key.
        """
        vtkinteractor = self._window.getVTKInteractor()
        vtkinteractor.SetKeySym(key)
        vtkinteractor.SetShiftKey(shift)
        vtkinteractor.InvokeEvent(vtk.vtkCommand.KeyPressEvent, vtkinteractor)
        vtkinteractor.SetKeySym(None)
        vtkinteractor.SetShiftKey(False)

    def moveMouse(self, x, y): #pylint: disable=invalid-name
        """
        Simulate a mouse movement.

        Inputs:
            x[float]: Position relative to the current window size in the horizontal direction.
            y[float]: Position relative to the current window size in the vertical direction.
        """

        # Determine relative position
        sz = self._window.getVTKWindow().GetSize()
        pos = [sz[0] * x, sz[1] * y]

        # Move the mouse
        vtkinteractor = self._window.getVTKInteractor()
        vtkinteractor.SetEventPosition(int(pos[0]), int(pos[1]))
        vtkinteractor.InvokeEvent(vtk.vtkCommand.MouseMoveEvent, vtkinteractor)

    def pressLeftMouseButton(self):
        """
        Simulate a left mouse click.
        """
        vtkinteractor = self._window.getVTKInteractor()
        vtkinteractor.InvokeEvent(vtk.vtkCommand.LeftButtonPressEvent, vtkinteractor)
