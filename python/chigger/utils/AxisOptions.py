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
from Options import Options

def get_options():
    """
    Retuns options for vtkAxis objects.
    """
    opt = Options()
    opt.add('num_ticks', 5, "The number of tick marks to place on the axis.", vtype=int)
    opt.add('lim', "The axis extents.", vtype=list)
    opt.add('color', [1, 1, 1], "The color of the axis, ticks, and labels.")
    opt.add('title', "The axis label.", vtype=str)
    opt.add('font_size', "The axis title and label font sizes, in points.", vtype=int)
    opt.add('title_font_size', "The axis title font size, in points.", vtype=int)
    opt.add('tick_font_size', "The axis tick label font size, in points.", vtype=int)
    opt.add('grid', True, "Show/hide the grid lines for this axis.")
    opt.add('grid_color', [0.25, 0.25, 0.25], "The color for the grid lines.")
    opt.add('precision', 3, "The axis numeric precision.", vtype=int)
    opt.add('notation', "The type of notation, leave empty to let VTK decide", vtype=str,
            allow=['standard', 'scientific', 'fixed', 'printf'])
    opt.add('ticks_visible', True, "Control visibility of tickmarks on colorbar axis.")
    opt.add('axis_visible', True, "Control visibility of axis line on colorbar axis.")
    opt.add('labels_visible', True, "Control visibility of the numeric labels.")
    opt.add('axis_position', 'left', "Set the axis position (left, right, top, bottom)", vtype=str,
            allow=['left', 'right', 'top', 'bottom'])
    opt.add('axis_point1', [0, 0], 'Starting location of axis, in absolute viewport coordinates.')
    opt.add('axis_point2', [0, 0], 'Ending location of axis, in absolute viewport coordinates.')
    opt.add('axis_scale', 1, "The axis scaling factor.")
    return opt

def set_options(vtkaxis, opt):
    """
    Set the options for vtkAxis object.
    """

    # Visibility
    vtkaxis.SetTicksVisible(opt['ticks_visible'])
    vtkaxis.SetAxisVisible(opt['axis_visible'])
    vtkaxis.SetLabelsVisible(opt['labels_visible'])

    # Ticks
    if opt.isOptionValid('num_ticks'):
        vtkaxis.SetNumberOfTicks(opt['num_ticks'])

    # Limits
    if opt.isOptionValid('lim'):
        vtkaxis.SetBehavior(vtk.vtkAxis.FIXED)
        vtkaxis.SetRange(*opt['lim'])
        vtkaxis.RecalculateTickSpacing()
    else:
        vtkaxis.SetBehavior(vtk.vtkAxis.AUTO)

    # Color
    if opt.isOptionValid('color'):
        clr = opt['color']
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
    vtkaxis.SetPrecision(opt['precision'])
    if opt.isOptionValid('notation'):
        notation = opt['notation'].upper()
        vtk_notation = getattr(vtk.vtkAxis, notation + '_NOTATION')
        vtkaxis.SetNotation(vtk_notation)

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

    if opt.isOptionValid('axis_scale'):
        vtkaxis.SetScalingFactor(opt['axis_scale'])
