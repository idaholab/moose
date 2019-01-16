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
from .. import utils
from .. import base

class TextAnnotationSource(base.ChiggerSourceBase):
    """
    Source for creating text annotations.
    """

    @staticmethod
    def getOptions():
        """
        Return default options for this object.
        """
        opt = base.ChiggerSourceBase.getOptions()
        opt.add('position', [0.5, 0.5], "The text position within the viewport, in relative "
                                        "coordinates.", vtype=tuple)
        opt.add('opacity', 1, "Set the text opacity.", vtype=float)
        opt += utils.FontOptions.get_options()
        return opt

    def __init__(self, **kwargs):
        super(TextAnnotationSource, self).__init__(vtkactor_type=vtk.vtkActor2D,
                                                   vtkmapper_type=vtk.vtkTextMapper, **kwargs)
        self._vtkactor.GetPositionCoordinate().SetCoordinateSystemToNormalizedViewport()

    def update(self, **kwargs):
        """
        Updates the settings for the text creation. (override)
        """
        super(TextAnnotationSource, self).update(**kwargs)
        utils.FontOptions.set_options(self._vtkmapper.GetTextProperty(), self._options)
        self._vtkactor.GetPositionCoordinate().SetValue(*self.getOption('position'))

        if self.isOptionValid('text'):
            self._vtkmapper.GetTextProperty().Modified()
            self._vtkmapper.SetInput(self.getOption('text'))

        if self.isOptionValid('opacity'):
            self._vtkmapper.GetTextProperty().Modified()
            self._vtkmapper.GetTextProperty().SetOpacity(self.getOption('opacity'))
