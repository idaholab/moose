#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html
class Color(object):
    """A Class to handle RGB color values"""

    def __init__(self, *args):
        if (len(args) == 1) and isinstance(args[0], Color):
            self._rgb = args[0].rgb()
        elif (len(args) == 3) and all([v >= 0 and v <= 1 for v in args]):
            self._rgb = args
        else:
            raise Exception('WRONG')
            self._rgb = None

    def rgb(self):
        """Return the RGB values"""
        return self._rgb

    def __ne__(self, other):
        return self._rgb != getattr(other, 'rgb', lambda: None)()

    def __eq__(self, other):
        return self._rgb == getattr(other, 'rgb', lambda: None)()


class AutoColor(Color):
    """A Class to handle RGB color values that operate with background color automatically."""


def auto_adjust_color(parent, children):
    """
    Helper function for automatically adjusting colors for the background.

    Inputs:
        parent[Window|Viewport]: Object with background settings
        children[ChiggerSourceBase]: Source objects with color settings

    see Window.py Viewport.py
    """
    def update_color_param(param, c):
        if (param.vtype is not None) and (AutoColor in param.vtype) and (param.value is None):
            param.setValue(c)
        elif param.value.__class__.__name__ == 'ChiggerInputParameters': # use name to avoid circular import
            for sub_param in param.value.parameters():
                update_color_param(sub_param, c)

    if parent.isParamValid('background', 'color') and (not parent.isParamValid('background', 'color2')):
        bg = parent.getParam('background', 'color')
        color = Color(1., 1., 1.) if sum(bg.rgb()) < 1.5 else Color(0., 0., 0.)
        for child in children:
            for param in child.parameters().parameters():
                update_color_param(param, color)
