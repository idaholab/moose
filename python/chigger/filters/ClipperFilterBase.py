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
from .ChiggerFilterBase import ChiggerFilterBase

class ClipperFilterBase(ChiggerFilterBase):
    """
    Base class for making clipping objects for ExodusSource objects.
    """
    CLIPFUNCTION_TYPE = vtk.vtkImplicitFunction

    @staticmethod
    def getOptions():
        opt = ChiggerFilterBase.getOptions()
        opt.add('normalized', True, "When True supplied position arguments are supplied in "
                                    "normalized coordinates (0-1) with respect to the object "
                                    "bounding box.")
        opt.add('inside_out', False, "When True the clipping criteria is reversed "
                                     "(see vtkClipDataSet::SetInsideOut)")
        return opt

    def __init__(self, vtkclipfunction=None, **kwargs):
        super(ClipperFilterBase, self).__init__(vtkfilter_type=vtk.vtkClipDataSet, **kwargs)

        self._vtkclipfunction = vtkclipfunction()
        self._vtkfilter.SetClipFunction(self._vtkclipfunction)

    def update(self, **kwargs):
        """
        Set the inside out status of the clipping data.
        """
        super(ClipperFilterBase, self).update(**kwargs)

        if self.isOptionValid('inside_out'):
            self._vtkfilter.SetInsideOut(self.getOption('inside_out'))

    def getPosition(self, position):
        """
        A method for converting the supplied position from normalized to global coordinates.

        Args:
            position[list]: The position to convert, note nothing occurs if 'normalized=False'.
        """
        if self._source.needsUpdate():
            self._source.update()
        bounds = self._source.getBounds()
        normalized = self.getOption('normalized')
        if normalized:
            for i in range(3):
                scale = bounds[1][i] - bounds[0][i]
                position[i] = position[i]*scale + bounds[0][i]
        return position
