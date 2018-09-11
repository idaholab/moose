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
import chigger

class PeacockObserver(chigger.observers.ChiggerObserver):
    """
    A special ChiggerObserver object for manipulating the color bar within a Peacock render window.
    """

    def __init__(self, *args, **kwargs):
        super(PeacockObserver, self).__init__(*args, **kwargs)
        self.__colorbar = None # when active this contains the colorbar
        self.__offset = [0, 0] # offset to keep the mouse in the same relative position on bar

    def init(self, *args, **kwargs):
        """
        Create necessary observers for Peacock result object selection.
        """
        super(PeacockObserver, self).init(*args, **kwargs)
        self._window.getVTKInteractor().AddObserver(vtk.vtkCommand.LeftButtonPressEvent,
                                                    self._onLeftButtonPressEvent)

        self._window.getVTKInteractor().AddObserver(vtk.vtkCommand.MouseMoveEvent,
                                                    self._onMouseMoveEvent)

    def _onLeftButtonPressEvent(self, obj, event):
        """
        Callback for LeftButtonPressEvent.
        """
        cbar = self._findResult(chigger.misc.ColorBar)
        if self.__colorbar is not None:
            self.__colorbar = None

        elif cbar is not None:
            loc = obj.GetEventPosition()
            bnds = cbar.getBounds()

            if (loc[0] > bnds[0] and loc[0] < bnds[1]) and (loc[1] > bnds[2] and loc[1] < bnds[3]):
                self._window.setActive(cbar)
                self.__colorbar = cbar

                sz = self._window.getVTKWindow().GetSize()
                origin = self.__colorbar.getOption('colorbar_origin')
                self.__offset[0] = float(loc[0])/float(sz[0]) - origin[0]
                self.__offset[1] = float(loc[1])/float(sz[1]) - origin[1]

    def _onMouseMoveEvent(self, obj, event):
        """
        Callback for MouseMoveEvent.
        """
        if self.__colorbar is not None:
            sz = self._window.getVTKWindow().GetSize()
            loc = list(obj.GetEventPosition())

            loc[0] = float(loc[0])/float(sz[0])# - self.__offset[0]
            loc[1] = float(loc[1])/float(sz[1])# - self.__offset[1]

            self.__colorbar.setOption('colorbar_origin', tuple(loc))
            self._window.update()

    def _findResult(self, rtype):
        """Helper for finding ChiggerResult objects."""
        out = None
        for result in self._window._results:
            if isinstance(result, rtype):
                out = result
        return out
