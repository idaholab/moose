#pylint: disable=missing-docstring
#################################################################
#                   DO NOT MODIFY THIS HEADER                   #
#  MOOSE - Multiphysics Object Oriented Simulation Environment  #
#                                                               #
#            (c) 2010 Battelle Energy Alliance, LLC             #
#                      ALL RIGHTS RESERVED                      #
#                                                               #
#           Prepared by Battelle Energy Alliance, LLC           #
#             Under Contract No. DE-AC07-05ID14517              #
#              With the U. S. Department of Energy              #
#                                                               #
#              See COPYRIGHT for full restrictions              #
#################################################################
from ChiggerObserver import ChiggerObserver
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
        super(TimerObserver, self).__init__(**kwargs)
        self._count = 0

    def init(self, window):
        """
        Add the TimerEvent for this object.
        """
        super(TimerObserver, self).init(window)
        window.getVTKInteractor().AddObserver('TimerEvent', self._callback)
        window.getVTKInteractor().CreateRepeatingTimer(self.getOption('duration'))

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
