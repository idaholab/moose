#pylint: disable=missing-docstring
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html
from TimerObserver import TimerObserver
class WindowUpdateObserver(TimerObserver):
    """
    A repeating timer that updates the RenderWindow.
    """
    @staticmethod
    def validParams():
        opt = TimerObserver.validParams()
        return opt

    def onTimer(self, obj, event): #pylint: disable=unused-argument
        """
        Updates the RenderWindow object.

        Inputs:
            obj, event: Required by VTK.
        """
        self._window.update()
