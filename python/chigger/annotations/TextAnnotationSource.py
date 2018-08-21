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
    def validOptions():
        """
        Return default options for this object.
        """
        opt = base.ChiggerSourceBase.validOptions()
        opt.add('position', default=(0.5, 0.5), vtype=float, size=2,
                doc="The text position within the viewport, in relative coordinates.")
        opt.add('text', vtype=str, doc="The text to display.")
        opt += utils.FontOptions.validOptions()
        return opt

    def __init__(self, **kwargs):
        super(TextAnnotationSource, self).__init__(vtkactor_type=vtk.vtkTextActor,
                                                   vtkmapper_type=None, **kwargs)
        self._vtkactor.GetPositionCoordinate().SetCoordinateSystemToNormalizedViewport()

    def update(self, **kwargs):
        """
        Updates the settings for the text creation. (override)
        """
        super(TextAnnotationSource, self).update(**kwargs)

        utils.FontOptions.applyOptions(self._vtkactor.GetTextProperty(), self._options)
        if self.isOptionValid('position'):
            self._vtkactor.GetPositionCoordinate().SetValue(*self.applyOption('position'))

        if self.isOptionValid('text'):
            self._vtkactor.SetInput(self.applyOption('text'))
