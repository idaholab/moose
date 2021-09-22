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
from moosetools import mooseutils
from .ChiggerInputParameters import ChiggerInputParameters

VTK_NOTATION_ENUM = [
    vtk.vtkAxis.STANDARD_NOTATION,
    vtk.vtkAxis.SCIENTIFIC_NOTATION,
    vtk.vtkAxis.FIXED_NOTATION,
    vtk.vtkAxis.PRINTF_NOTATION
]

def validParams():
    """Returns options for vtkAxis objects."""
    opt = ChiggerInputParameters()
    opt.add('fontcolor', default=(1,1,1), vtype=(int, float), size=3, doc="The color of the axis, ticks, and labels.")
    opt.add('axis_fontcolor', vtype=(int, float), size=3, doc="The color of the axis, this overrides the value in 'fontcolor'.")

    return opt

    # opt.add('num_ticks', default=5, vtype=int, doc="The number of tick marks to place on the axis.")
    # opt.add('lim', vtype=(int, float), size=2, doc="The axis extents.")
    # opt.add('font_color', (1, 1, 1), vtype=(int, float), size=3,
    #         doc="The color of the axis, ticks, and labels.")
    # opt.add('title', vtype=str, doc="The axis label.")
    # opt.add('font_size', vtype=int, doc="The axis title and label font sizes, in points.")
    # opt.add('title_font_size', vtype=int, doc="The axis title font size, in points.")
    # opt.add('tick_font_size', vtype=int, doc="The axis tick label font size, in points.")
    # opt.add('grid', True, vtype=bool, doc="Show/hide the grid lines for this axis.")
    # opt.add('grid_color', default=(0.25, 0.25, 0.25), vtype=(int, float), size=3,
    #         doc="The color for the grid lines.")
    # opt.add('precision', 3, vtype=int, doc="The axis numeric precision.")
    # opt.add('notation', vtype=str, doc="The type of notation, leave empty to let VTK decide.",
    #         allow=('standard', 'scientific', 'fixed', 'printf'))
    # opt.add('ticks_visible', True, vtype=bool,
    #         doc="Control visibility of tickmarks on colorbar axis.")
    # opt.add('axis_visible', True, vtype=bool,
    #         doc="Control visibility of axis line on colorbar axis.")
    # opt.add('labels_visible', True, vtype=bool, doc="Control visibility of the numeric labels.")
    # opt.add('axis_position', 'left', vtype=str,
    #         doc="Set the axis position (left, right, top, bottom)",
    #         allow=('left', 'right', 'top', 'bottom'))
    # opt.add('axis_point1', (0, 0), vtype=(int, float), size=2,
    #         doc='Starting location of axis, in absolute viewport coordinates.')
    # opt.add('axis_point2', (0, 0), vtype=(int, float), size=2,
    #         doc='Ending location of axis, in absolute viewport coordinates.')
    # opt.add('axis_scale', 1, vtype=(int, float), doc="The axis scaling factor.")
    # opt.add('zero_tol', 1e-10, vtype=(int, float), doc="Tolerance for considering limits to be the same.")

def setParams(vtkaxis, opt): #pylint: disable=invalid-name
    """
    Set the options for vtkAxis object.
    """


    pass

    """
    # Visibility
    if opt.isValid('tick_visible'):
        vtkaxis.SetTicksVisible(opt.applyOption('ticks_visible'))

    if opt.isValid('axis_visible'):
        vtkaxis.SetAxisVisible(opt.applyOption('axis_visible'))

    if opt.isValid('labels_visible'):
        vtkaxis.SetLabelsVisible(opt.applyOption('labels_visible'))

    # Ticks
    if opt.isValid('num_ticks'):
        vtkaxis.SetNumberOfTicks(opt.applyOption('num_ticks'))

    # Limits
    if opt.isValid('lim'):
        lim = opt.get('lim')
        if abs(lim[1] - lim[0]) < opt.get('zero_tol'):
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
    """

    # Color
    if opt.isValid('fontcolor'):
        clr = opt.getValue('fontcolor')
        vtkaxis.GetTitleProperties().SetColor(*clr)
        vtkaxis.GetLabelProperties().SetColor(*clr)
        vtkaxis.GetPen().SetColorF(*clr)

    """
    # Axis title
    if opt.isValid('title'):
        vtkaxis.SetTitle(opt.applyOption('title'))

    # Font sizes
    if opt.isValid('font_size'):
        vtkaxis.GetTitleProperties().SetFontSize(opt.applyOption('font_size'))
        vtkaxis.GetLabelProperties().SetFontSize(opt.applyOption('font_size'))

    if opt.isValid('title_font_size'):
        vtkaxis.GetTitleProperties().SetFontSize(opt.applyOption('title_font_size'))
    if opt.isValid('tick_font_size'):
        vtkaxis.GetLabelProperties().SetFontSize(opt.applyOption('tick_font_size'))

    # Precision/notation
    if opt.isValid('precision'):
        vtkaxis.SetPrecision(opt.applyOption('precision'))

    if opt.isValid('notation'):
        notation = opt.get('notation').upper()
        vtk_notation = getattr(vtk.vtkAxis, notation + '_NOTATION')
        vtkaxis.SetNotation(vtk_notation)

    if opt.isValid('precision'):
        if vtkaxis.GetNotation() in VTK_NOTATION_ENUM[1:2]:
            vtkaxis.SetPrecision(opt['precision'])
        else:
            mooseutils.mooseWarning("When using 'precision' option, 'notation' option has to be "
                                    "set to either 'scientific' or 'fixed'.")

    # Grid lines
    if opt.isValid('grid'):
        vtkaxis.SetGridVisible(opt.applyOption('grid'))

    if opt.isValid('grid_color'):
        vtkaxis.GetGridPen().SetColorF(opt.applyOption('grid_color'))

    # Set the position and points
    if opt.isValid('axis_position'):
        pos = {'left':vtk.vtkAxis.LEFT, 'right':vtk.vtkAxis.RIGHT, 'top':vtk.vtkAxis.TOP,
               'bottom':vtk.vtkAxis.BOTTOM}
        vtkaxis.SetPosition(pos[opt.applyOption('axis_position')])

    if opt.isValid('axis_point1'):
        vtkaxis.SetPoint1(*opt.applyOption('axis_point1'))

    if opt.isValid('axis_point2'):
        vtkaxis.SetPoint2(*opt.applyOption('axis_point2'))

    if opt.isValid('axis_scale'):
        vtkaxis.SetScalingFactor(opt.applyOption('axis_scale'))
    """
