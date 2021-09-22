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
from Line import Line
from .. import base
from .. import utils

class Graph(base.ChiggerResultBase):
    """
    A class for creating XY line or point graphs.
    """

    @staticmethod
    def validParams():
        """
        Params specific to the graph as a whole. (static)
        """
        opt = base.ChiggerResultBase.validParams()
        opt.add('color_scheme', vtype=str, doc="The VTK color scheme to utilize.")
        opt.add('xaxis', default=utils.AxisParams.validParams(),
                doc="The settings for the x-axis.")
        opt.add('yaxis', default=utils.AxisParams.validParams(),
                doc="The settings for the y-axis.")
        opt.add('x2axis', default=utils.AxisParams.validParams(),
                doc="The settings for the secondary x-axis (top).")
        opt.add('y2axis', default=utils.AxisParams.validParams(),
                doc="The settings for the secondary y-axis (right).")
        opt.add('legend', default=utils.LegendParams.validParams(),
                doc="The settings for the graph legend.")
        opt.add('hidden_border', vtype=int,
                doc="Adjust the border of hidden axis; useful if axis text is cut-off.")
        opt.add('lines', default=[], doc="A list of Line objects to display on the graph.")
        opt.add('font_size', default=12, vtype=int,
                doc="The font size for all aspects of the chart.")

        # Remove position from axis options, it should not be available b/c this is set by the graph
        for ax in ['xaxis', 'yaxis', 'x2axis', 'y2axis']:
            opt.getValue(ax).remove('axis_position')

        return opt

    @staticmethod
    def validKeyBindings():
        bindings = base.ChiggerResult.validKeyBindings()
        bindings.add('f', Graph._setFont, desc="Increase the font size by 1 point.")
        bindings.add('f', Graph._setFont, shift=True, desc="Decrease the font size by 1 point.")
        bindings.add('l', Graph._setLineThickness, desc="Increase the line thickness by 1 point.")
        bindings.add('l', Graph._setLineThickness, shift=True,
                     desc="Decrease the line thickness by 1 point.")
        return bindings

    def __init__(self, *args, **kwargs):
        super(Graph, self).__init__(**kwargs)

        # Create a view actor, this is where the plots are displayed
        self._vtkchart = vtk.vtkChartXY()
        self._vtkview = vtk.vtkContextActor()
        self._vtkview.GetScene().AddItem(self._vtkchart)
        self._vtkrenderer.AddViewProp(self._vtkview)

        # vtkColorSeries for automatic line colors
        self._vtkcolorseries = None

        # Add the ChiggerPlot objects
        self._plots = []

        # Add the lines provided as arguments
        lines = self.getParam('lines') if self.isValid('lines') else []
        lines.extend(args)
        self.setParam('lines', lines)

    def update(self, *args, **kwargs):
        """
        Update method for the graph axes.
        """
        super(Graph, self).update(*args, **kwargs)

        # Global font_size
        if self.isValid('font_size'):
            fz = self.applyOption('font_size')
            for sub_opt in ['xaxis', 'yaxis', 'x2axis', 'y2axis', 'legend']:
                opt = self.getParam(sub_opt)
                if not opt.isValid('font_size'):
                    opt.setValue('font_size', fz)

        # Update Axis options
        self._setAxisParams(self._vtkchart.GetAxis(vtk.vtkAxis.BOTTOM), self.getParam('xaxis'))
        self._setAxisParams(self._vtkchart.GetAxis(vtk.vtkAxis.LEFT), self.getParam('yaxis'))
        self._setAxisParams(self._vtkchart.GetAxis(vtk.vtkAxis.TOP), self.getParam('x2axis'))
        self._setAxisParams(self._vtkchart.GetAxis(vtk.vtkAxis.RIGHT), self.getParam('y2axis'))

        # Legend
        utils.LegendParams.setParams(self._vtkchart, self._vtkrenderer, self.getParam('legend'))

        # Line objects
        if self.isValid('lines'):
            for line in self._parameters.get('lines'):
                self._addPlotObject(line)
            self._parameters.remove('lines')

        # Current vtkPlot objects to display
        current = set()
        for line in self._plots:
            current.update(line.getVTKPlotObjects())

        # Error if no line objects exists
        if len(self._plots) == 0:
            mooseutils.mooseError("No line objects exists, thus no graph axes was created.")
            return

        # vtkPlot objects that are being replaced
        old = set()
        for i in range(self._vtkchart.GetNumberOfPlots()):
            old.add(self._vtkchart.GetPlot(i))

        # Remove vtkPlot objects that are in old, but not in current
        for plot in old.difference(current):
            idx = self._vtkchart.GetPlotIndex(plot)
            self._vtkchart.RemovePlot(idx)

        # Add the new plots
        for plot in current.difference(old):
            self._vtkchart.AddPlot(plot)

        # Set plot corners
        for line in self._plots:
            corner = line.options().raw('corner').allow.index(line.getParam('corner'))
            for plot in line.getVTKPlotObjects():
                self._vtkchart.SetPlotCorner(plot, corner)

            line.update()

        # Adjust hidden borders
        if self.isValid('hidden_border'):
            self._vtkchart.SetHiddenAxisBorder(self.getParam('hidden_border'))

    def _addPlotObject(self, line):
        """
        A helper method for inserting a line, handling color automatically, into the graph.

        Args:
            line[chigger.graph.Line]: The line object to add to this graph.
        """

        # Initialize the line (this creates the vtk object)
        line.initialize()

        # Set the line color, if not set by the user
        if not line.isValid('color'):

            # Colors
            if self._vtkcolorseries is None:
                self._vtkcolorseries = vtk.vtkColorSeries()
                if self.isValid('color_scheme'):
                    scheme = eval('vtk.vtkColorSeries.' + self.getParam('color_scheme').upper())
                    self._vtkcolorseries.SetColorScheme(scheme)

            n_lines = len(self._plots)
            n_colors = self._vtkcolorseries.GetNumberOfColors()
            if n_lines >= n_colors:
                mooseutils.mooseWarning('The number of lines exceeds the number of available '
                                        'line colors.')

            c = self._vtkcolorseries.GetColorRepeating(n_lines)
            c = [c[0]/256., c[1]/256., c[2]/256.]

            # If the color matches the background, flip it
            b = self.getRenderWindow().getParam('background')
            if (c[0] == b[0]) and (c[1] == b[1]) and (c[2] == c[2]):
                c[0] = 1 - c[0]
                c[1] = 1 - c[1]
                c[2] = 1 - c[2]

            line.setParam('color', tuple(c))

        self._plots.append(line)

    def _setAxisParams(self, vtkaxis, opt):
        """
        Helper method for updating vtkAxis settings. (protected)

        Args:
            axis[vtkAxis]: The axis to update.
            opt[Params]: The options to apply to the axis.
        """
        utils.AxisParams.setParams(vtkaxis, opt)

        # Adjust hidden border size (changes made here are overridden by the 'hidden_border' option)
        if self._vtkchart.GetHiddenAxisBorder() < vtkaxis.GetLabelProperties().GetFontSize():
            self._vtkchart.SetHiddenAxisBorder(vtkaxis.GetLabelProperties().GetFontSize())

    def _setFont(self, window, binding): #pylint: disable=unused-argument
        """Keybinding method."""
        step = -1 if binding.shift else 1
        sz = self.getParam('font_size') + step
        self.update(font_size=sz)
        self.printOption('font_size')

    def _setLineThickness(self, window, binding): #pylint: disable=unused-argument
        """Keybinding method."""
        increment = -1 if binding.shift else 1
        for plt in self._plots:
            if isinstance(plt, Line):
                sz = plt.getParam('width') + increment
                if sz >= 0:
                    plt.update(width=sz)
                plt.printOption('width')
