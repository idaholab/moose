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
from ChiggerObserver import ChiggerObserver
class SingleShotObserver(ChiggerObserver):
    """
    Class for creating timers to be passed in to RenderWindow object.
    """
    @staticmethod
    def validParams():
        opt = ChiggerObserver.validParams()
        opt.add('duration', default=1000, vtype=int,
                doc="Trigger delay, in milliseconds, of the single shot timer.")
        opt.add('terminate', default=False, vtype=bool,
                doc="Terminate the VTK window when the 'count' is reached.")

        return opt

    def init(self, *args, **kwargs):
        """
        Add a repeating timer.
        """
        super(SingleShotObserver, self).init(*args, **kwargs)
        self._window.getVTKInteractor().CreateOneShotTimer(self.getParam('duration'))
        self._window.getVTKInteractor().AddObserver(vtk.vtkCommand.TimerEvent, self._callback)

    def onTimer(self, obj, event): #pylint: disable=no-self-use, unused-argument
        """The 'onTimer(obj, event)' method must be implemented."""
        return None

    def _callback(self, obj, event): #pylint: disable=unused-argument
        """
        The function to be called by the RenderWindow.

        Inputs:
            obj, event: Required by VTK.
        """
        self.onTimer(obj, event)
        if self.getParam('terminate'):
            self._window.getVTKInteractor().TerminateApp()
