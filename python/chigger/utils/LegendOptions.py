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
from .Options import Options

def get_options():
    """
    Options specific to the legend.
    """
    opt = Options()
    opt.add('visible', True, "Control the visibility of the legend.")
    opt.add('background', "Set the legend background color (defaults to graph background color).",
            vtype=list)
    opt.add('opacity', 0, "The legend background opacity.")
    opt.add('label_color', [1, 1, 1], "The legend text color.")
    opt.add('label_font_size', "The legend label test size in points.", vtype=int)
    opt.add('point', "The location of the legend anchor point.", vtype=list)
    opt.add('horizontal_alignment', 'right', "The horizontal alignment of the legend with respect "
                                             "to the anchor point.",
            vtype=str, allow=['left', 'center', 'right'])
    opt.add('vertical_alignment', 'top', "The vertical alignment of the legend with respect to the "
                                         "anchor point.",
            vtype=str, allow=['top', 'bottom', 'center'])
    opt.add('border', False, "Show the legend border.")
    opt.add('border_color', [1, 1, 1], "The border color.")
    opt.add('border_opacity', 1, "The border opacity.")
    opt.add('border_width', "The border width.", vtype=float)
    return opt

def set_options(vtkchart, vtkrenderer, opt):
    """
    A method for updating the legend options.
    """

    legend = vtkchart.GetLegend()

    if opt.isOptionValid('visible'):
        vtkchart.SetShowLegend(opt['visible'])
    else:
        vtkchart.SetShowLegend(True)

    if opt.isOptionValid('background'):
        legend.GetBrush().SetColorF(opt['background'])
    else:
        legend.GetBrush().SetColorF(vtkrenderer.GetBackground())

    legend.GetLabelProperties().SetColor(opt['label_color'])
    legend.GetBrush().SetOpacityF(opt['opacity'])

    if opt.isOptionValid('label_font_size'):
        legend.SetLabelSize(opt['label_font_size'])

    if opt.isOptionValid('point'):
        pt = opt['point']
        legend.SetVerticalAlignment(vtk.vtkChartLegend.CUSTOM)
        legend.SetHorizontalAlignment(vtk.vtkChartLegend.CUSTOM)

        coord = vtk.vtkCoordinate()
        coord.SetCoordinateSystemToNormalizedViewport()
        coord.SetValue(pt[0], pt[1], 0)
        loc = coord.GetComputedViewportValue(vtkrenderer)
        legend.SetPoint(*loc)
    else:
        legend.SetVerticalAlignment(eval('vtk.vtkChartLegend.' +
                                         opt['vertical_alignment'].upper()))
        legend.SetHorizontalAlignment(eval('vtk.vtkChartLegend.' +
                                           opt['horizontal_alignment'].upper()))

    if opt.isOptionValid('border'):
        legend.GetPen().SetOpacity(opt['border_opacity'])
        legend.GetPen().SetColorF(opt['border_color'])
        if opt.isOptionValid('border_width'):
            legend.GetPen().SetWidth(opt['border_width'])
    else:
        legend.GetPen().SetOpacity(0)
