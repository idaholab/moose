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
from ChiggerObject import ChiggerObject
class ChiggerTimer(ChiggerObject):
    """
    Class for creating timers to be passed in to RenderWindow.start() method.

    Inputs:
        window[chigger.base.RenderWindow]: The RenderWindow object that will execute the timer.
    """

    @staticmethod
    def getOptions():
        opt = ChiggerObject.getOptions()
        opt.add('duration', 1000, "The repeat interval, in milliseconds, of the timer.", vtype=int)
        opt.add('count', None, "The maximum number of timer calls before terminating timer loop.",
                vtype=int)
        opt.add('terminate', False, "Terminate the VTK window when the 'count' is reached.")
        return opt

    def __init__(self, window, **kwargs):
        super(ChiggerTimer, self).__init__(**kwargs)
        self._count = 0
        self._window = window

    def duration(self):
        """
        Return the timer duration (ms)
        """
        return self.getOption('duration')

    def count(self):
        """
        Return the current number of callback calls.
        """
        return self._count

    def callback(self, obj, event): #pylint: disable=unused-argument
        """
        The function to be called by the RenderWindow.

        Inputs:
            obj, event: Required by VTK.
        """
        if self.needsUpdate():
            self.update()

        if self.isOptionValid('count') and (self._count >= self.getOption('count')):
            self._window.getVTKInteractor().DestroyTimer()
            if self.getOption('terminate'):
                self._window.getVTKInteractor().TerminateApp()
            return
        self._window.update()
        self._count += 1
