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
from .. import base
from .. import utils

class Graph(base.ChiggerResultBase):
    """
    A class for creating XY line or point graphs.
    """

    @staticmethod
    def getOptions():
        """
        Options specific to the graph as a whole. (static)
        """
        opt = base.ChiggerResultBase.getOptions()
        opt.add('color_scheme', "The VTK color scheme to utilize.", vtype=str)
        opt.add('xaxis', utils.AxisOptions.get_options(), "The settings for the x-axis.")
        opt.add('yaxis', utils.AxisOptions.get_options(), "The settings for the y-axis.")
        opt.add('x2axis', utils.AxisOptions.get_options(), "The settings for the secondary "
                                                           "x-axis (top).")
        opt.add('y2axis', utils.AxisOptions.get_options(), "The settings for the secondary "
                                                           "y-axis (right).")
        opt.add('legend', utils.LegendOptions.get_options(), "The settings for the graph legend.")
        opt.add('hidden_border', None, "Adjust the border of hidden axis; useful if axis text is "
                                       "cut-off.", vtype=int)
        opt.add('lines', [], "A list of Line objects to display on the graph.")

        # Remove position from axis options, it should not be available b/c this is set by the graph
        for ax in ['xaxis', 'yaxis', 'x2axis', 'y2axis']:
            opt[ax].pop('axis_position')

        return opt

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
        lines = self.getOption('lines') if self.isOptionValid('lines') else []
        lines.extend(args)
        self.setOption('lines', lines)

    def needsUpdate(self):
        """
        Checks if the Graph or child lines need to be updated.
        """
        return super(Graph, self).needsUpdate() or any([p.needsUpdate() for p in self._plots])

    def update(self, *args, **kwargs):
        """
        Update method for the graph axes.
        """
        super(Graph, self).update(*args, **kwargs)

        # Update Axis options
        self._setAxisOptions(self._vtkchart.GetAxis(vtk.vtkAxis.BOTTOM), self.getOption('xaxis'))
        self._setAxisOptions(self._vtkchart.GetAxis(vtk.vtkAxis.LEFT), self.getOption('yaxis'))
        self._setAxisOptions(self._vtkchart.GetAxis(vtk.vtkAxis.TOP), self.getOption('x2axis'))
        self._setAxisOptions(self._vtkchart.GetAxis(vtk.vtkAxis.RIGHT), self.getOption('y2axis'))

        # Legend
        utils.LegendOptions.set_options(self._vtkchart, self._vtkrenderer, self.getOption('legend'))

        # Line objects
        if self.isOptionValid('lines'):
            for line in self._options.pop('lines'):
                self._addPlotObject(line)

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
            corner = line.options().raw('corner').allow.index(line.getOption('corner'))
            for plot in line.getVTKPlotObjects():
                self._vtkchart.SetPlotCorner(plot, corner)

            if line.needsUpdate():
                line.update()

        # Adjust hidden borders
        if self.isOptionValid('hidden_border'):
            self._vtkchart.SetHiddenAxisBorder(self.getOption('hidden_border'))

    def _addPlotObject(self, line):
        """
        A helper method for inserting a line, handling color automatically, into the graph.

        Args:
            line[chigger.graph.Line]: The line object to add to this graph.
        """

        # Initialize the line (this creates the vtk object)
        if line.needsInitialize():
            line.initialize()

        # Set the line color, if not set by the user
        if not line.isOptionValid('color'):

            # Colors
            if self._vtkcolorseries is None:
                self._vtkcolorseries = vtk.vtkColorSeries()
                if self.isOptionValid('color_scheme'):
                    scheme = eval('vtk.vtkColorSeries.' + self.getOption('color_scheme').upper())
                    self._vtkcolorseries.SetColorScheme(scheme)

            n_lines = len(self._plots)
            n_colors = self._vtkcolorseries.GetNumberOfColors()
            if n_lines >= n_colors:
                mooseutils.mooseWarning('The number of lines exceeds the number of available '
                                        'line colors.')

            c = self._vtkcolorseries.GetColorRepeating(n_lines)
            b = self.getOption('background')

            # If the color matches the background, flip it
            if (c[0] == b[0]) and (c[1] == b[1]) and (c[2] == c[2]):
                c[0] = 1 - c[0]
                c[1] = 1 - c[1]
                c[2] = 1 - c[2]

            line.setOption('color', c)

        self._plots.append(line)

    def _setAxisOptions(self, vtkaxis, opt):
        """
        Helper method for updating vtkAxis settings. (protected)

        Args:
            axis[vtkAxis]: The axis to update.
            opt[Options]: The options to apply to the axis.
        """
        utils.AxisOptions.set_options(vtkaxis, opt)

        # Adjust hidden border size (changes made here are overridden by the 'hidden_border' option)
        if self._vtkchart.GetHiddenAxisBorder() < vtkaxis.GetLabelProperties().GetFontSize():
            self._vtkchart.SetHiddenAxisBorder(vtkaxis.GetLabelProperties().GetFontSize())
