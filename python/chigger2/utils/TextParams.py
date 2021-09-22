#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html
from .AutoColor import Color, AutoColor
from .ChiggerInputParameters import ChiggerInputParameters

def validParams():
    """Returns options for vtkTextProperty."""
    opt = ChiggerInputParameters()
    opt.add('rotate', default=0., vtype=(int, float),
            verify=(lambda v: v>=0 and v<360, "The supplied value must in range [0,360)"),
            doc="The text rotation in degrees.")
    opt.add('halign', default='left', vtype=str, allow=('left', 'center', 'right'),
            doc="Set the font justification.")
    opt.add('valign', default='bottom', allow=('bottom', 'center', 'top'),
            doc="The vertical text justification.")

    # font_...
    font = ChiggerInputParameters()
    font.add('color', vtype=(AutoColor, Color), doc="The text color.")
    font.add('opacity', default=1., vtype=(int, float),
            verify=(lambda v: v>=0 and v<=1, "The supplied value must be in range [0,1]"),
            doc="The text opacity.")
    font.add('size', default=0.05, vtype=(int, float),
             verify=(lambda v: v>0 and v<=1, "The supplied size must be in range (0,1]"),
             doc="The text font size in relative viewport (vertical) coordinates")
    font.add('italic', default=False, vtype=bool, doc="Use italics style text")
    font.add('bold', default=False, vtype=bool, doc="Use bold style text")
    font.add('family', default='arial', vtype=str, allow=('arial', 'courier', 'times'),
             doc="The font family")
    opt.add('font', default=font, vtype=ChiggerInputParameters, doc="Font options")

    # frame_...
    frame = ChiggerInputParameters()
    frame.add('on', vtype=bool, default=False, doc="Enable the text frame")
    frame.add('color', vtype=(int, float), size=3, doc="The color of the frame around text, defaults to text color")
    frame.add('width', default=1, vtype=int,
              doc="The width of the frame around text")
    opt.add('frame', default=frame, vtype=ChiggerInputParameters, doc="Frame options")

    # background_...
    bg = ChiggerInputParameters()
    bg.add('color', vtype=Color, doc="The color of the text background.")
    bg.add('opacity', default=1., vtype=(int, float),
            verify=(lambda v: v>0 and v<=1, "The supplied value must be in range (0,1]"),
            doc="The opacity of the text background.")
    opt.add('background', default=bg, vtype=ChiggerInputParameters, doc="Background options")

    return opt

def applyParams(actor, viewport, tprop, opt):
    """
    Applies font options to vtkTextProperty object.

    Inputs:
        tprop: A vtk.vtkTextProperty object for applying options.
        options: The Params object containing the settings to apply.
    """
    # Do nothing if text does not exist
    if not actor.GetInput():
        return

    # This makes the rendered font fill the entire region (try it, set font_size = 1)
    tprop.SetUseTightBoundingBox(True)


    # FONT
    font = opt.getValue('font')
    if font.isValid('color'):
        tprop.SetColor(font.getValue('color').rgb())
    font.assign('opacity', tprop.SetOpacity)
    font.assign('italic', tprop.SetItalic)
    font.assign('bold', tprop.SetBold)
    family = font.getValue('family')
    if family == 'arial':
        tprop.SetFontFamilyToArial()
    elif family == 'courier':
        tprop.SetFontFamilyToCourier()
    elif family == 'times':
        tprop.SetFontFamilyToTimes()
    else:
        # This isn't reachable if using the properties above
        raise ValueError("The supplied font family of '{}' was not recognized.".format(family))

    # Set up a reasonable estimate of the font size based on the viewport relative dimensions.
    # The vtkTextActor has support for scaling to the viewport, but I could not get it to work
    # like I would expect. This is a suitable replacement and it seems to render correct.

    # Set the font to a fixed font size and determine its size
    size = int(32)
    tprop.SetFontSize(size)
    tprop.SetOrientation(0)
    font_pixels = [0]*2
    actor.GetSize(viewport, font_pixels)

    # Compute a scaling factor based on height
    view_pixels = viewport.GetSize()
    w = font_pixels[0] / view_pixels[0]
    h = font_pixels[1] / view_pixels[1]
    scale = font.getValue('size') / h

    # Apply scaled size and rotation
    tprop.SetFontSize(int(scale*size))
    tprop.SetOrientation(opt.getValue('rotate'))

    # FRAME
    frame = opt.getValue('frame')
    frame.assign('on', tprop.SetFrame)
    frame.assign('color', tprop.SetFrameColor)
    frame.assign('width', tprop.SetFrameWidth)

    # BACKGROUND
    bg = opt.getValue('background')
    if bg.isValid('color'):
        bg.assign('color', tprop.SetBackgroundColor)
        bg.assign('opacity', tprop.SetBackgroundOpacity)
    else:
        tprop.SetBackgroundOpacity(0)

    opt.assign('rotate', tprop.SetOrientation)

    halign = opt.getValue('halign')
    if halign == 'left':
        tprop.SetJustificationToLeft()
    if halign == 'center':
        tprop.SetJustificationToCentered()
    if halign == 'right':
        tprop.SetJustificationToRight()

    valign = opt.getValue('valign')
    if valign == 'bottom':
        tprop.SetVerticalJustificationToBottom()
    if valign == 'center':
        tprop.SetVerticalJustificationToCentered()
    if valign == 'top':
        tprop.SetVerticalJustificationToTop()
