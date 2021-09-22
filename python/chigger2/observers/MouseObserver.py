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
class MouseObserver(ChiggerObserver):
    """
    Class for creating mouse move observers to be passed in to RenderWindow object.
    """

    @staticmethod
    def validParams():
        opt = ChiggerObserver.validParams()
        return opt

    def __init__(self, **kwargs):
        super(MouseObserver, self).__init__(**kwargs)

    def init(self, *args):
        """
        Add the MouseMoveEvent for this object.
        """
        super(MouseObserver, self).init(*args)
        return self._window.getVTKInteractor().AddObserver(vtk.vtkCommand.MouseMoveEvent,
                                                           self.__callback)

    def onMouseMove(self, pos, obj, event):
        raise NotImplementedError("The 'onMouseMove(pos, obj, event)' method must be implemented.")

    def __callback(self, obj, event): #pylint: disable=unused-argument
        """
        The function to be called by the RenderWindow.

        Inputs:
            obj, event: Required by VTK.
        """
        result = self._window.getActive()
        if result is not None:
            loc = obj.GetEventPosition()
            sz = result.getVTKRenderer().GetSize()
            position = (loc[0]/float(sz[0]), loc[1]/float(sz[1]))
            self.onMouseMove(position, obj, event)
