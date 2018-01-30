#pylint: disable=missing-docstring
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html
from chigger.base import ChiggerObject
class ChiggerObserver(ChiggerObject):
    """
    Base class for definning VTK observer functions.

    This object is a base class and not intended for general use, see TimerObserver as an example.
    """
    @staticmethod
    def getOptions():
        opt = ChiggerObject.getOptions()
        return opt

    def __init__(self, event, **kwargs):
        super(ChiggerObserver, self).__init__(**kwargs)
        self._event = event
        self._window = None
        self._vtk_command = None

    def addObserver(self, event, vtkinteractor):
        """
        Add the observer to the supplied vtkInteractor, see TimerObserver for an example.

        Generally, this method is not needed. If you are creating a new Observer it should inherit
        from one of the existing objects: e.g., KeyObserver, TimerObserver.

        Inputs:
            event[str]: The VTK event string, this is supplied in the constructor
            vtkinteractor[vtkInteractor]: The interactor that the observer should be added.

        NOTE: This method must return the VTK id return from the AddObserver method.
        """
        raise NotImplementedError("The addObserver method must be implmented.")

    def isActive(self):
        """
        Return True if this observer is already a part of the VTK interactor.
        """
        if self._vtk_command is not None:
            return self._window.getVTKInteractor().HasObserver(self._event, self._vtk_command)
        else:
            return False

    def init(self, window):
        """
        Initialize the observer, this is called by the RenderWindow automatically.

        NOTE: This is an internal method, do not call it.
        """
        self._window = window
        vtkid = self.addObserver(self._event, self._window.getVTKInteractor())
        self._vtk_command = window.getVTKInteractor().GetCommand(vtkid)
