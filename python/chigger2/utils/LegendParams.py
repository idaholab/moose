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
from .ChiggerInputParameters import ChiggerInputParameters

def validParams(): #pylint: disable=invalid-name
    """
    Params specific to the legend.
    """
    opt = ChiggerInputParameters()
    opt.add('visible', default=True, vtype=bool,
            doc="Control the visibility of the legend.")
    opt.add('background', vtype=(int, float), size=3,
            doc="Set the legend background color (defaults to graph background color).")
    opt.add('opacity', default=0, vtype=(int, float), doc="The legend background opacity.")
    opt.add('color', default=(1, 1, 1), size=3, vtype=(int, float), doc="The legend text color.")
    opt.add('font_size', vtype=int, doc="The legend label test size in points.")
    opt.add('point', size=2, vtype=(int, float), doc="The location of the legend anchor point.")
    opt.add('horizontal_alignment', default='right', vtype=str, allow=('left', 'center', 'right'),
            doc="The horizontal alignment of the legend with respect to the anchor point.")
    opt.add('vertical_alignment', default='top', vtype=str, allow=('top', 'center', 'bottom'),
            doc="The vertical alignment of the legend with respect to the anchor point.")
    opt.add('border', default=False, vtype=bool, doc="Show the legend border.")
    opt.add('border_color', default=(1, 1, 1), size=3, vtype=(int, float), doc="The border color.")
    opt.add('border_width', doc="The border width.", vtype=(int, float))
    return opt

def setParams(vtkchart, vtkrenderer, opt): #pylint: disable=invalid-name
    """
    A method for updating the legend options.
    """

    legend = vtkchart.GetLegend()

    if opt.isValid('visible'):
        vtkchart.SetShowLegend(opt.applyOption('visible'))

    #if opt.isValid('background'):
    #    legend.GetBrush().SetColorF(opt.applyOption('background'))
    #else:
    #    legend.GetBrush().SetColorF(vtkrenderer.GetBackground())

    if opt.isValid('color'):
        legend.GetLabelProperties().SetColor(opt.applyOption('color'))

    if opt.isValid('opacity'):
        legend.GetBrush().SetOpacityF(opt.applyOption('opacity'))

    if opt.isValid('font_size'):
        legend.SetLabelSize(opt.applyOption('font_size'))

    if opt.isValid('point'):
        pt = opt.getValue('point')
        legend.SetVerticalAlignment(vtk.vtkChartLegend.CUSTOM)
        legend.SetHorizontalAlignment(vtk.vtkChartLegend.CUSTOM)

        coord = vtk.vtkCoordinate()
        coord.SetCoordinateSystemToNormalizedViewport()
        coord.SetValue(pt[0], pt[1], 0)
        loc = coord.GetComputedViewportValue(vtkrenderer)
        legend.SetPoint(*loc)
    else:
        legend.SetVerticalAlignment(eval('vtk.vtkChartLegend.' +
                                         opt.getValue('vertical_alignment').upper()))
        legend.SetHorizontalAlignment(eval('vtk.vtkChartLegend.' +
                                           opt.getValue('horizontal_alignment').upper()))

    if opt.getValue('border'):
        if opt.isValid('border_color'):
            legend.GetPen().SetColorF(opt.applyOption('border_color'))
        if opt.isValid('border_width'):
            legend.GetPen().SetWidth(opt.applyOption('border_width'))
    else:
        legend.GetPen().SetOpacity(0)
