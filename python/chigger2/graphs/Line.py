#pylint: disable=missing-docstring
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import vtki
from moosetools import mooseutils
from .. import base

class Line(base.ChiggerObject):
    """
    Wrapper for vtk line/point object.
    """

    @staticmethod
    def validParams():
        opt = base.ChiggerObject.validParams()
        opt.add('x', vtype=list, doc="The x-axis data.")
        opt.add('y', vtype=list, doc="The y-axis data.")
        opt.add('label', vtype=str, doc="The plot label (name appearing in legend).")
        opt.add('style', default='-', vtype=str, allow=('none', '-'), doc="The line style.")
        opt.add('color', vtype=(int, float), size=3, doc="The color of the line to plot.")
        opt.add('width', vtype=int, default=1, doc="The width of the line in Points.")
        opt.add('corner', default='left-bottom', vtype=str,
                allow=('left-bottom', 'right-bottom', 'right-top', 'left-top'),
                doc="The axis corner to place the line.")
        opt.add('marker', default='none', doc="Set the marker type.",
                allow=('none', 'cross', 'plus', 'square', 'circle', 'diamond'))
        opt.add('append', default=True, vtype=bool, doc="Append new data to the existing data.")
        opt.add('tracer', default=False, vtype=bool,
                doc="Places both x and y tracing lines, (see 'xtracer' and 'ytracer').")
        opt.add('xtracer', vtype=bool,
                doc="Place a tracing line that follows the leading x-value (overrides "
                    "'tracer' option).")
        opt.add('ytracer', vtype=bool,
                doc="Place a tracing line that follows the leading y-value (overrides "
                    "'tracer' option).")

        return opt

    def __init__(self, x_data=None, y_data=None, **kwargs):
        super(Line, self).__init__(**kwargs)

        # Storage for vtk line/point object
        self._vtkplot = None

        # Build the vtkTable that stores the data
        x = vtk.vtkFloatArray()
        x.SetName('x-data')
        y = vtk.vtkFloatArray()
        y.SetName('y-data')
        self._vtktable = vtk.vtkTable()
        self._vtktable.AddColumn(x)
        self._vtktable.AddColumn(y)

        # Storage for tracing lines
        self._xtracer = None
        self._ytracer = None

        # Set x,y data
        if x_data:
            self.setParam('x', x_data)
        if y_data:
            self.setParam('y', y_data)

    def setParams(self, *args, **kwargs):
        """
        Update line objects settings.
        """
        super(Line, self).setParams(*args, **kwargs)

        tracer = self.getParam('tracer')
        if tracer and not self.isValid('xtracer'):
            self.setParam('xtracer', True)
        if tracer and not self.isValid('ytracer'):
            self.setParam('ytracer', True)

    def initialize(self):
        """
        Called prior to inserting the vtkPlotLine/Points object into the chart.

        see Graph::Update
        """
        # Create the vtk line or points object
        style = self.getParam('style')
        if style == '-' and not isinstance(self._vtkplot, vtk.vtkPlotLine):
            self._vtkplot = vtk.vtkPlotLine()
            self._vtkplot.SetInputData(self._vtktable, 0, 1)

        elif style is 'none' and not isinstance(self._vtkplot, vtk.vtkPlotPoints):
            self._vtkplot = vtk.vtkPlotPoints()
            self._vtkplot.SetInputData(self._vtktable, 0, 1)

        # Create tracer lines(s)
        if self.getParam('xtracer'):
            if self._xtracer is None:
                self._xtracer = Line(append=False, width=0.1, color=self.getParam('color'))
                self._xtracer.initialize()

        if self.getParam('ytracer'):
            if self._ytracer is None:
                self._ytracer = Line(append=False, width=0.1, color=self.getParam('color'))
                self._ytracer.initialize()

    def getVTKPlot(self):
        """
        Return the vtkPlot object for this line.
        """
        return self._vtkplot

    def update(self, **kwargs):
        """
        Update the line object because of data of settings changed.
        """
        super(Line, self).update(**kwargs)

        # Remove x,y data
        if not self.getParam('append'):
            self._vtktable.SetNumberOfRows(0)

        # Get the x,y data and reset to None so that data doesn't append over and over
        x = self.getParam('x')
        y = self.getParam('y')
        self._parameters.setValue('x', None)
        self._parameters.setValue('y', None)
        if (x and y) and (len(x) == len(y)):
            for i in range(len(x)): #pylint: disable=consider-using-enumerate
                array = vtk.vtkVariantArray()
                array.SetNumberOfTuples(2)
                array.SetValue(0, x[i])
                array.SetValue(1, y[i])
                self._vtktable.InsertNextRow(array)
            self._vtktable.Modified()

        elif (x and y) and (len(x) != len(y)):
            mooseutils.MooseException("Supplied x and y data must be same length.")

        # Apply the line/point settings
        if self.isValid('color'):
            self._vtkplot.SetColor(*self.getParam('color'))

        if self.isValid('width'):
            self._vtkplot.SetWidth(self.getParam('width'))

        if self.isValid('label'):
            self._vtkplot.SetLabel(self.getParam('label'))

        vtk_marker = getattr(vtk.vtkPlotLine, self.getParam('marker').upper())
        self._vtkplot.SetMarkerStyle(vtk_marker)

        # Label
        if not self.isValid('label'):
            self._vtkplot.LegendVisibilityOff()
        else:
            self._vtkplot.LegendVisibilityOn()

        # Handle single point data
        if self._vtktable.GetNumberOfRows() == 1:
            self._vtktable.InsertNextRow(self._vtktable.GetRow(0))

        # Tracers
        if self._xtracer:
            ax = self._vtkplot.GetYAxis()
            rmin = ax.GetMinimum()
            rmax = ax.GetMaximum()
            value = self._vtktable.GetValue(self._vtktable.GetNumberOfRows()-1, 0)
            self._xtracer.update(x=[value, value], y=[rmin, rmax])

        if self._ytracer:
            ax = self._vtkplot.GetXAxis()
            rmin = ax.GetMinimum()
            rmax = ax.GetMaximum()
            value = self._vtktable.GetValue(self._vtktable.GetNumberOfRows()-1, 1)
            self._ytracer.update(x=[rmin, rmax], y=[value, value])

    def getVTKPlotObjects(self):
        """
        Return the vtkPlotLine/vtkPlotPoints object.

        see Graph.py
        """
        objects = [self._vtkplot]

        if self._xtracer:
            objects.append(self._xtracer.getVTKPlot())
        if self._ytracer:
            objects.append(self._ytracer.getVTKPlot())

        return objects
