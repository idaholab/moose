#pylint: disable=missing-docstring
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from ImageAnnotationSource import ImageAnnotationSource
from .. import base

class ImageAnnotation(base.ChiggerResult):
    """
    Result object for displaying images in 3D space.

    Inputs:
        key,value pairs set the options for this object.
    """

    @staticmethod
    def validOptions():
        """
        Return the default options for this object.
        """
        opt = base.ChiggerResult.validOptions()
        opt += ImageAnnotationSource.validOptions()
        return opt

    @staticmethod
    def validKeyBindings():
        bindings = base.ChiggerResult.validKeyBindings()
        bindings.add('w', ImageAnnotation._setWidth,
                     desc="Increase the scale of the image by 0.01.")
        bindings.add('w', ImageAnnotation._setWidth, shift=True,
                     desc="Decrease the scale of the image by 0.01.")
        bindings.add('a', ImageAnnotation._setOpacity,
                     desc="Increase the opacity (alpha) of the image by 0.05.")
        bindings.add('a', ImageAnnotation._setOpacity, shift=True,
                     desc="Decrease the opacity (alpha) of the image by 0.05.")
        return bindings

    def __init__(self, **kwargs):
        super(ImageAnnotation, self).__init__(ImageAnnotationSource(), **kwargs)

    def setActive(self, active):
        """
        Overrides the default active highlighting.
        """
        if active:
            self._sources[0].getVTKActor().GetProperty().SetBackingColor(1, 0, 0)
            self._sources[0].getVTKActor().GetProperty().SetBacking(True)

        else:
            self._sources[0].getVTKActor().GetProperty().SetBacking(False)

    def onMouseMoveEvent(self, position):
        """
        Re-position the image based on the mouse position.
        """
        self.setOption('position', position)
        self.printOption('position')

    def _setWidth(self, window, binding): #pylint: disable=unused-argument
        """
        Callback for setting the image width.
        """
        step = -0.01 if binding.shift else 0.01
        width = self.getOption('width') + step
        if width > 0 and (width <= 1):
            self.setOption('width', width)
            self.printOption('width')

    def _setOpacity(self, window, binding): #pylint: disable=unused-argument
        """
        Callback for changing opacity.
        """
        step = -0.05 if binding.shift else 0.05
        opacity = self.getOption('opacity') + step
        if opacity > 0 and opacity < 1:
            self.setOption('opacity', opacity)
            self.printOption('opacity')
