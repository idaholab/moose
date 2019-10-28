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
import mooseutils
from .Options import Options

VTK_NOTATION_ENUM = [
    vtk.vtkAxis.STANDARD_NOTATION,
    vtk.vtkAxis.SCIENTIFIC_NOTATION,
    vtk.vtkAxis.FIXED_NOTATION,
    vtk.vtkAxis.PRINTF_NOTATION
]

def get_options():
    """
    Retuns options for vtkAxis objects.
    """
    opt = Options()
    opt.add('num_ticks', 5, "The number of tick marks to place on the axis.", vtype=int)
    opt.add('lim', "The axis extents.", vtype=list)
    opt.add('font_color', [1, 1, 1], "The color of the axis, ticks, and labels.")
    opt.add('title', "The axis label.", vtype=str)
    opt.add('font_size', "The axis title and label font sizes, in points.", vtype=int)
    opt.add('title_font_size', "The axis title font size, in points.", vtype=int)
    opt.add('tick_font_size', "The axis tick label font size, in points.", vtype=int)
    opt.add('grid', True, "Show/hide the grid lines for this axis.")
    opt.add('grid_color', [0.25, 0.25, 0.25], "The color for the grid lines.")
    opt.add('precision', "The axis numeric precision.", vtype=int)
    opt.add('notation', "The type of notation, leave empty to let VTK decide", vtype=str,
            allow=['standard', 'scientific', 'fixed', 'printf'])
    opt.add('ticks_visible', True, "Control visibility of tickmarks on colorbar axis.")
    opt.add('axis_visible', True, "Control visibility of axis line on colorbar axis.")
    opt.add('labels_visible', True, "Control visibility of the numeric labels.")
    opt.add('axis_position', 'left', "Set the axis position (left, right, top, bottom)", vtype=str,
            allow=['left', 'right', 'top', 'bottom'])
    opt.add('axis_point1', [0, 0], 'Starting location of axis, in absolute viewport coordinates.')
    opt.add('axis_point2', [0, 0], 'Ending location of axis, in absolute viewport coordinates.')
    opt.add('axis_scale', 1, "The axis scaling factor.", vtype=float)
    opt.add('axis_factor', 0, "Offset the axis by adding a factor.", vtype=float)
    opt.add('axis_opacity', 1, "The vtkAxis opacity.", vtype=float)
    opt.add('zero_tol', 1e-10, "Tolerance for considering limits to be the same.")
    return opt

def set_options(vtkaxis, opt):
    """
    Set the options for vtkAxis object.
    """

    # Visibility
    vtkaxis.SetTicksVisible(opt['ticks_visible'])
    vtkaxis.SetAxisVisible(opt['axis_visible'])
    vtkaxis.SetLabelsVisible(opt['labels_visible'])

    # Opacity
    if opt.isOptionValid('axis_opacity'):
        opacity = opt['axis_opacity']
        vtkaxis.SetOpacity(opacity)
        vtkaxis.GetTitleProperties().SetOpacity(opacity)
        vtkaxis.GetLabelProperties().SetOpacity(opacity)

    # Ticks
    if opt.isOptionValid('num_ticks'):
        vtkaxis.SetNumberOfTicks(opt['num_ticks'])

    # Limits
    if opt.isOptionValid('lim'):
        lim = opt['lim']
        if abs(lim[1] - lim[0]) < opt['zero_tol']:
            vtkaxis.SetBehavior(vtk.vtkAxis.CUSTOM)
            vtkaxis.SetRange(0, 1)

            pos = vtk.vtkDoubleArray()
            pos.SetNumberOfTuples(2)
            pos.SetValue(0, 0)
            pos.SetValue(1, 1)

            labels = vtk.vtkStringArray()
            labels.SetNumberOfTuples(2)
            labels.SetValue(0, str(lim[0]))
            labels.SetValue(1, str(lim[1]))

            vtkaxis.SetCustomTickPositions(pos, labels)
        else:
            vtkaxis.SetCustomTickPositions(None, None)
            vtkaxis.SetBehavior(vtk.vtkAxis.FIXED)
            scale = opt['axis_scale']
            factor = opt['axis_factor']
            vtkaxis.SetRange(lim[0] * scale + factor, lim[1] * scale + factor)
            vtkaxis.RecalculateTickSpacing()
    else:
        vtkaxis.SetBehavior(vtk.vtkAxis.AUTO)
        vtkaxis.SetCustomTickPositions(None, None)

    # Color
    if opt.isOptionValid('font_color'):
        clr = opt['font_color']
        vtkaxis.GetTitleProperties().SetColor(*clr)
        vtkaxis.GetLabelProperties().SetColor(*clr)
        vtkaxis.GetPen().SetColorF(*clr)

    # Axis title
    if opt.isOptionValid('title'):
        vtkaxis.SetTitle(opt['title'])

    # Font sizes
    if opt.isOptionValid('font_size'):
        vtkaxis.GetTitleProperties().SetFontSize(opt['font_size'])
        vtkaxis.GetLabelProperties().SetFontSize(opt['font_size'])

    if opt.isOptionValid('title_font_size'):
        vtkaxis.GetTitleProperties().SetFontSize(opt['title_font_size'])
    if opt.isOptionValid('tick_font_size'):
        vtkaxis.GetLabelProperties().SetFontSize(opt['tick_font_size'])

    # Precision/notation
    if opt.isOptionValid('notation'):
        notation = opt['notation'].upper()
        vtk_notation = getattr(vtk.vtkAxis, notation + '_NOTATION')
        vtkaxis.SetNotation(vtk_notation)

    if opt.isOptionValid('precision'):
        if vtkaxis.GetNotation() in VTK_NOTATION_ENUM[1:3]:
            vtkaxis.SetPrecision(opt['precision'])
        else:
            mooseutils.mooseWarning("When using 'precision' option, 'notation' option has to be "
                                    "set to either 'scientific' or 'fixed'.")

    # Grid lines
    vtkaxis.SetGridVisible(opt['grid'])
    vtkaxis.GetGridPen().SetColorF(opt['grid_color'])

    # Set the position and points
    if opt.isOptionValid('axis_position'):
        pos = {'left':vtk.vtkAxis.LEFT, 'right':vtk.vtkAxis.RIGHT, 'top':vtk.vtkAxis.TOP,
               'bottom':vtk.vtkAxis.BOTTOM}
        vtkaxis.SetPosition(pos[opt['axis_position']])

    if opt.isOptionValid('axis_point1'):
        vtkaxis.SetPoint1(*opt['axis_point1'])

    if opt.isOptionValid('axis_point2'):
        vtkaxis.SetPoint2(*opt['axis_point2'])
