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
