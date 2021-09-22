#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html
from .AutoColor import AutoColor, Color
from .ChiggerInputParameters import ChiggerInputParameters

def validParams():
    """Returns options for vtkTextProperty."""
    opt = ChiggerInputParameters()

    bg = ChiggerInputParameters()
    bg.add('color', vtype=(Color, AutoColor), doc="The primary background color")
    bg.add('color2', vtype=Color, doc="The secondary background color, when specified a gradient background is enabled")
    bg.add('opacity', default=0, vtype=(int, float),
           verify=(lambda v: v>=0 and v <= 1, "The 'opacity' must be in the range [0,1]"),
           doc="The background opacity")
    opt.add('background', default=bg, vtype=ChiggerInputParameters, doc="Background options")
    return opt

def applyParams(viewport, opt):
    """
    Applies background options to vtkViewport object.

    Inputs:
        viewport: A vtk.vtkViewport object for applying options.
        opt: The Params object containing the settings to apply.
    """
    opt.assign('opacity', viewport.SetBackgroundAlpha)
    if opt.isValid('color'):
        viewport.SetBackground(*opt.getValue('color').rgb())
    if opt.isValid('color2'):
        viewport.SetBackground2(*opt.getValue('color2').rgb())
        viewport.SetGradientBackground(True)
    else:
        viewport.SetGradientBackground(False)
