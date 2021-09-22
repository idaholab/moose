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
class KeyObserver(ChiggerObserver):
    """
    Class for creating key press observers to be passed in to RenderWindow object.
    """

    @staticmethod
    def validParams():
        opt = ChiggerObserver.validParams()
        return opt

    def __init__(self, **kwargs):
        super(KeyObserver, self).__init__(**kwargs)

    def init(self, *args):
        """
        Add the KeyPressEvent for this object.
        """
        super(KeyObserver, self).init(*args)
        return self._window.getVTKInteractor().AddObserver(vtk.vtkCommand.KeyPressEvent,
                                                           self.__callback)

    def onKeyPress(self, key, shift, obj, event):
        raise NotImplementedError("The 'onKeyPress(key, obj, event)' method must be implemented.")

    def __callback(self, obj, event): #pylint: disable=unused-argument
        """
        The function to be called by the RenderWindow.

        Inputs:
            obj, event: Required by VTK.
        """
        self.onKeyPress(obj.GetKeySym(), obj.GetShiftKey(), obj, event)
