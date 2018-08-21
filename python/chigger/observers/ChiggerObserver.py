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
    Base class for defining VTK observer functions.

    This object is a base class and not intended for general use, see TimerObserver as an example.
    """
    @staticmethod
    def validOptions():
        opt = ChiggerObject.validOptions()
        return opt

    def __init__(self, **kwargs):
        super(ChiggerObserver, self).__init__(**kwargs)
        self._window = None

    def init(self, window):
        """
        Initialize the observer, this is called by the RenderWindow automatically.

        NOTE: This is an internal method, do not call it.
        """
        self._window = window

    def terminate(self):
        """
        Terminate the render window.
        """
        self._window.getVTKInteractor().TerminateApp()
