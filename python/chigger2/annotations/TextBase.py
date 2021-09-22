#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html
import vtk
import math
from .Annotation import Annotation
from .. import utils
class TextBase(Annotation):
    """
    Base for text based annotations.
    """
    VTKACTORTYPE = vtk.vtkTextActor

    @staticmethod
    def validParams():
        opt = Annotation.validParams()
        opt += utils.TextParams.validParams()
        return opt

    @staticmethod
    def validKeyBindings():
        bindings = Annotation.validKeyBindings()
        bindings.add('f', TextBase._incrementFont, args=(0.05,),
                     desc="Increase the font size by 1 point.")
        bindings.add('f', TextBase._incrementFont, args=(-0.05,), shift=True,
                     desc="Decrease the font size by 1 point.")
        bindings.add('a', TextBase._incrementAlpha, args=(0.2,),
                     desc="Increase the font alpha (opacity) by 2%.")
        bindings.add('a', TextBase._incrementAlpha, args=(-0.2,), shift=True,
                     desc="Decrease the font alpha (opacity) by 2%.")
        bindings.add('r', TextBase._incrementRotate, args=(2,),
                     desc="Rotate by two degrees counter clockwise.")
        bindings.add('r', TextBase._incrementRotate, args=(-2,), shift=True,
                     desc="Rotate by two degrees counter clockwise.")
        return bindings

    def __init__(self, *args, **kwargs):
        Annotation.__init__(self, *args, **kwargs)
        self._vtkmapper = self._vtkactor.GetMapper()
        self._vtkactor.GetPositionCoordinate().SetCoordinateSystemToNormalizedViewport()
        self._vtkactor.GetPosition2Coordinate().SetCoordinateSystemToNormalizedViewport()

    def _onRequestInformation(self, *args):
        self.assignParam('position', self._vtkactor.SetPosition)
        utils.TextParams.applyParams(self._vtkactor, self._viewport.getVTKRenderer(),
                                       self._vtkactor.GetTextProperty(), self._parameters)

        # Do this late so that the highlight function overrides the current frame settings
        Annotation._onRequestInformation(self, *args)

    #def _highlight(self):
    #    if self.getParam('highlight'):
    #        self._vtkactor.GetTextProperty().SetFrame(True)
    #        self._vtkactor.GetTextProperty().SetFrameColor((1,1,0))
    #        self._vtkactor.GetTextProperty().SetFrameWidth(3)

    def _incrementFont(self, delta):
        font = self.getParam('font')
        sz = font.getValue('size') + delta
        font.setValue(size, sz)
        self.printOption('font')

    def _incrementAlpha(self, delta):
        opacity = self.getParam('opacity') + delta
        if opacity <= 1.:
            self.setParams(opacity=opacity)
            self.printOption('opacity')

    def _incrementRotate(self, delta):
        angle = self.getParam('rotate') + delta
        if angle > 360:
            angle = angle - 360
        elif angle < 0:
            angle = angle + 360

        self.setParam('rotate', angle)
        self.printOption('rotate')
