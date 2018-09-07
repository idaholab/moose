#pylint: disable=missing-docstring
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html
#pylint: enable=missing-docstring

from chigger import base

class TextAnnotationBase(base.ChiggerResult):
    """
    Base for text based annotations.
    """
    @staticmethod
    def validOptions():
        opt = base.ChiggerResult.validOptions()
        opt.remove('camera')
        return opt

    @staticmethod
    def validKeyBindings():
        bindings = base.ChiggerResult.validKeyBindings()
        bindings.add('f', TextAnnotationBase._increaseFont,
                     desc="Increase the font size by 1 point.")
        bindings.add('f', TextAnnotationBase._decreaseFont, shift=True,
                     desc="Decrease the font size by 1 point.")
        bindings.add('a', TextAnnotationBase._increaseOpacity,
                     desc="Increase the font alpha (opacity) by 1%.")
        bindings.add('a', TextAnnotationBase._decreaseOpacity, shift=True,
                     desc="Decrease the font alpha (opacity) by 1%.")
        bindings.add('d', TextAnnotationBase._toggleMouseDrag,
                     desc="Enable/disable the ability to drag the text with the mouse.")
        return bindings

    def __init__(self, source, **kwargs):
        super(TextAnnotationBase, self).__init__(source, **kwargs)

        # TODO: Allow frame options, the highlight will need to remember the options
        for src in self.getSources():
            src.getVTKActor().GetTextProperty().SetFrameColor(1, 0, 0)
            src.getVTKActor().GetTextProperty().SetFrameWidth(2)

        # Flag for allowing mouse to move the text, this was needed because it was too easy
        # to mess up the next when toggling through result options.
        self._allow_mouse_drag = False

    def _toggleMouseDrag(self, *args): #pylint: disable=unused-argument
        """Keybinding method."""
        self._allow_mouse_drag = not self._allow_mouse_drag

        if self._allow_mouse_drag:
            for src in self.getSources():
                src.getVTKActor().GetTextProperty().SetFrameColor(1, 1, 0)
        else:
            for src in self.getSources():
                src.getVTKActor().GetTextProperty().SetFrameColor(1, 0, 0)

    def _increaseFont(self, *args): #pylint: disable=unused-argument
        """Keybinding method."""
        sz = self.getOption('font_size') + 1
        self.update(font_size=sz)
        self.printOption('font_size')

    def _decreaseFont(self, *args): #pylint: disable=unused-argument
        """Keybinding method."""
        sz = self.getOption('font_size') - 1
        self.update(font_size=sz)
        self.printOption('font_size')

    def _increaseOpacity(self, *args): #pylint: disable=unused-argument
        """Keybinding method."""
        opacity = self.getOption('text_opacity') + 0.01
        if opacity <= 1.:
            self.update(text_opacity=opacity)
            self.printOption('text_opacity')

    def _decreaseOpacity(self, *args): #pylint: disable=unused-argument
        """Keybinding method."""
        opacity = self.getOption('text_opacity') - 0.01
        if opacity > 0.:
            self.update(text_opacity=opacity)
            self.printOption('text_opacity')

    def onMouseMoveEvent(self, position):
        """Called by MainWindowObserver when the mouse moves and this object is active."""
        if self._allow_mouse_drag:
            self.update(position=position)
            self.printOption('position')

    def setActive(self, active):
        """Overrides the default active highlighting."""
        if self.getOption('highlight_active'):
            for src in self.getSources():
                src.getVTKActor().GetTextProperty().SetFrame(active)
