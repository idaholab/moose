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
from .ChiggerObserver import ChiggerObserver
class TimerObserver(ChiggerObserver):
    """
    Class for creating timers to be passed in to RenderWindow object.
    """
    @staticmethod
    def getOptions():
        opt = ChiggerObserver.getOptions()
        opt.add('duration', 1000, "The repeat interval, in milliseconds, of the timer.", vtype=int)
        opt.add('count', None, "The maximum number of timer calls before terminating timer loop.",
                vtype=int)
        opt.add('terminate', False, "Terminate the VTK window when the 'count' is reached.")
        return opt

    def __init__(self, **kwargs):
        super(TimerObserver, self).__init__(vtk.vtkCommand.TimerEvent, **kwargs)
        self._count = 0

    def addObserver(self, event, vtkinteractor):
        """
        Add the TimerEvent for this object.
        """
        vtkinteractor.CreateRepeatingTimer(self.getOption('duration'))
        return vtkinteractor.AddObserver(event, self._callback)

    def count(self):
        """
        Return the current number of callback calls.
        """
        return self._count

    def update(self, **kwargs):
        """
        Update the window object.
        """
        super(TimerObserver, self).update(**kwargs)
        if self._window.needsUpdate():
            self._window.update()

    def _callback(self, obj, event): #pylint: disable=unused-argument
        """
        The function to be called by the RenderWindow.

        Inputs:
            obj, event: Required by VTK.
        """
        if self.isOptionValid('count') and (self._count >= self.getOption('count')):
            self._window.getVTKInteractor().DestroyTimer()
            if self.getOption('terminate'):
                self._window.getVTKInteractor().TerminateApp()
            return
        self.update()
        self._count += 1
