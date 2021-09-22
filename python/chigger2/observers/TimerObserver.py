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
class TimerObserver(ChiggerObserver):
    """
    Class for creating timers to be passed in to RenderWindow object.
    """
    @staticmethod
    def validParams():
        opt = ChiggerObserver.validParams()
        opt.add('duration', default=1000, vtype=int,
                doc="The repeat interval, in milliseconds, of the timer.")
        opt.add('count', vtype=int,
                doc="The maximum number of timer calls before terminating timer loop.")
        opt.add('terminate', default=False, vtype=bool,
                doc="Terminate the VTK window when the 'count' is reached.")
        return opt

    def __init__(self, **kwargs):
        super(TimerObserver, self).__init__(**kwargs)
        self._count = 0

    def init(self, *args, **kwargs):
        """
        Add a repeating timer.
        """
        super(TimerObserver, self).init(*args, **kwargs)
        self._window.getVTKInteractor().CreateRepeatingTimer(self.getParam('duration'))
        self._window.getVTKInteractor().AddObserver(vtk.vtkCommand.TimerEvent, self._callback)

    def count(self):
        """
        Return the current number of callback calls.
        """
        return self._count

    def onTimer(self, obj, event):
        raise NotImplementedError("The 'onTimer(obj, event)' method must be implemented.")

    def _callback(self, obj, event): #pylint: disable=unused-argument
        """
        The function to be called by the RenderWindow.

        Inputs:
            obj, event: Required by VTK.
        """
        if self.isValid('count') and (self._count >= self.getParam('count')):
            self._window.getVTKInteractor().DestroyTimer()
            if self.getParam('terminate'):
                self.terminate()
            return
        self._count += 1
        self.onTimer(obj, event)
