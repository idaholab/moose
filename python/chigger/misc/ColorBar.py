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
from .AxisSource import AxisSource
from .. import base
from .. import geometric


class ColorBar(base.ChiggerResult):
    """
    A generic colorbar.

    The default colorbar in VTK, vtkScalarBarActor, tightly couples the tick marks with the bar
    width, which makes it difficult to control either. This class decouples the colorbar and labels.
    """
    @staticmethod
    def getOptions():
        opt = base.ChiggerResult.getOptions()
        opt.add('location', 'right', "The location of the primary axis.",
                allow=['left', 'right', 'top', 'bottom'])
        opt.add('colorbar_origin', None, "The position of the colorbar, relative to the viewport.",
                vtype=tuple)
        opt.add('width', 0.05, "The width of the colorbar, relative to window.", vtype=float)
        opt.add('length', 0.5, "The height of the colorbar, relative to window.", vtype=float)
        opt += base.ColorMap.getOptions()

        ax0 = AxisSource.getOptions()
        ax0.setDefault('ticks_visible', False)
        ax0.setDefault('axis_visible', False)
        ax0.pop('color')
        opt.add('primary', ax0, "The primary axis options.")

        ax1 = AxisSource.getOptions()
        ax1.setDefault('axis_visible', False)
        ax1.setDefault('ticks_visible', False)
        ax1.setDefault('visible', False)
        ax1.pop('color')
        opt.add('secondary', ax1, "The secondary axis options.")
        return opt

    def __init__(self, **kwargs):
        super(ColorBar, self).__init__(**kwargs)
        self._sources = [geometric.PlaneSource2D(), AxisSource(), AxisSource()]

    def update(self, **kwargs):
        """
        Apply setting to create a colorbar.

        Inputs:
            see ChiggerResult
        """

        # Set the options provided
        self.setOptions(**kwargs)
        if self.needsInitialize():
            self.initialize()

        # Convenience names for the various sources
        plane, axis0, axis1 = self._sources

        # Origin
        loc = self.getOption('location').lower()
        if not self.isOptionValid('colorbar_origin'):
            if (loc == 'right') or (loc == 'left'):
                self.setOption('colorbar_origin', [0.8, 0.25, 0.0])
            else:
                self.setOption('colorbar_origin', [0.25, 0.2, 0.0])

        # Get dimensions of colorbar, taking into account the orientation
        n = self.getOption('cmap_num_colors')
        length0 = 0
        length1 = 0
        loc = self.getOption('location')
        if (loc == 'right') or (loc == 'left'):
            length0 = self.getOption('width')
            length1 = self.getOption('length')
            plane.setOptions(resolution=[1, n+1])
        else:
            length0 = self.getOption('length')
            length1 = self.getOption('width')
            plane.setOptions(resolution=[n+1, 1])

        # Coordinate system transformation object
        pos = self.getOption('colorbar_origin')
        coord = vtk.vtkCoordinate()
        coord.SetCoordinateSystemToNormalizedViewport()

        # The viewport must be set, before the points are computed.
        if self.isOptionValid('viewport'):
            self._vtkrenderer.SetViewport(self.getOption('viewport'))

        # Point 0
        coord.SetViewport(self._vtkrenderer)
        coord.SetValue(pos[0]+length0, pos[1], 0)
        p0 = coord.GetComputedViewportValue(self._vtkrenderer)

        # Point 1
        coord.SetValue(pos[0], pos[1]+length1, 0)
        p1 = coord.GetComputedViewportValue(self._vtkrenderer)

        coord.SetValue(*pos)
        pos = coord.GetComputedViewportValue(self._vtkrenderer)

        # Update the bar position
        plane.setOptions(origin=[pos[0], pos[1], 0],
                         point1=[p0[0], p0[1], 0],
                         point2=[p1[0], p1[1], 0])

        # Set the colormap for the bar
        rng = self.getOption('cmap_range')
        step = (rng[1] - rng[0]) / float(n)

        data = vtk.vtkFloatArray()
        data.SetNumberOfTuples(n+1)
        for i in range(n+1):
            data.SetValue(i, rng[0] + i*step)
        plane.setOptions(data=data)

        # Setup the primary Axis
        axis0.options().update(self.getOption('primary'))
        location = self.__setAxisPosition(axis0, p0, p1, self.getOption('location'))

        # Setup the secondary Axis
        axis1.options().update(self.getOption('secondary'))
        self.__setAxisPosition(axis1, p0, p1, location)

        # Call base class method
        super(ColorBar, self).update()

    @staticmethod
    def __setAxisPosition(axis, pt0, pt1, location):
        """
        Helper for setting the position of the axis along the color plane.
        """

        # The secondary location to output
        secondary_location = None

        # Position the axis
        axis.setOption('axis_position', location.lower())
        if location.lower() == 'left':
            axis.setOption('axis_point1', [pt1[0], pt0[1]])
            axis.setOption('axis_point2', [pt1[0], pt1[1]])
            secondary_location = 'right'

        elif location.lower() == 'right':
            axis.setOption('axis_point1', [pt0[0], pt0[1]])
            axis.setOption('axis_point2', [pt0[0], pt1[1]])
            secondary_location = 'left'

        elif location.lower() == 'top':
            axis.setOption('axis_point1', [pt1[0], pt1[1]])
            axis.setOption('axis_point2', [pt0[0], pt0[1]])
            secondary_location = 'bottom'

        elif location.lower() == 'bottom':
            axis.setOption('axis_point1', [pt1[0], pt0[1]])
            axis.setOption('axis_point2', [pt0[0], pt0[1]])
            secondary_location = 'top'
        return secondary_location
