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
from AxisSource import AxisSource
from .. import base
from .. import geometric


class ColorBar(base.ChiggerResult):
    """
    A generic colorbar.

    The default colorbar in VTK, vtkScalarBarActor, tightly couples the tick marks with the bar
    width, which makes it difficult to control either. This class decouples the colorbar and labels.
    """
    @staticmethod
    def validOptions():
        opt = base.ChiggerResult.validOptions()
        opt.add('location', 'right',
                doc="The location of the primary axis.",
                allow=('left', 'right', 'top', 'bottom'))
        opt.add('colorbar_origin',
                doc="The position of the colorbar, relative to the viewport.",
                vtype=float,
                size=2)
        opt.add('width',
                default=0.05,
                vtype=float,
                doc="The width of the colorbar, relative to window.")
        opt.add('length',
                default=0.5,
                vtype=float,
                doc="The height of the colorbar, relative to window.")
        opt += base.ColorMap.validOptions()

        ax0 = AxisSource.validOptions()
        ax0.set('ticks_visible', False)
        ax0.set('axis_visible', False)
        opt.add('primary', default=ax0, doc="The primary axis options.")

        ax1 = AxisSource.validOptions()
        ax1.set('axis_visible', False)
        ax1.set('ticks_visible', False)
        ax1.set('visible', False)
        opt.add('secondary', default=ax1, doc="The secondary axis options.")
        return opt

    @staticmethod
    def validKeyBindings():
        bindings = base.ChiggerResult.validKeyBindings()

        bindings.add('w', lambda s, *args: ColorBar._increment(s, 0.005, 'width', *args),
                     desc="Increase the width of the colorbar by 0.005.")
        bindings.add('w', lambda s, *args: ColorBar._increment(s, -0.005, 'width', *args), shift=True,
                     desc="Decrease the width of the colorbar by 0.005.")
        bindings.add('l', lambda s, *args: ColorBar._increment(s, 0.005, 'length', *args),
                     desc="Increase the length of the colorbar by 0.005.")
        bindings.add('l', lambda s, *args: ColorBar._increment(s, -0.005, 'length', *args), shift=True,
                     desc="Decrease the length of the colorbar by 0.005.")
        bindings.add('f', lambda s, *args: ColorBar._incrementFont(s, 1),
                     desc="Increase the font size by 1 point (when result is selected).")
        bindings.add('f', lambda s, *args: ColorBar._incrementFont(s, -1), shift=True,
                     desc="Decrease the font size by 1 point (when result is selected).")
        return bindings

    def __init__(self, **kwargs):
        super(ColorBar, self).__init__(geometric.PlaneSource2D(),
                                       AxisSource(),
                                       AxisSource(),
                                       **kwargs)

    def update(self, **kwargs):
        """
        Apply setting to create a colorbar.

        Inputs:
            see ChiggerResult
        """
        # Call base class method
        super(ColorBar, self).update(**kwargs)

        # Convenience names for the various sources
        plane, axis0, axis1 = self._sources

        # Origin
        loc = self.getOption('location').lower()
        if not self.isOptionValid('colorbar_origin'):
            if (loc == 'right') or (loc == 'left'):
                self.setOption('colorbar_origin', (0.8, 0.25))
            else:
                self.setOption('colorbar_origin', (0.25, 0.2))

        # Get dimensions of colorbar, taking into account the orientation
        n = self.getOption('cmap_num_colors')
        length0 = 0
        length1 = 0
        loc = self.getOption('location')
        if (loc is 'right') or (loc is 'left'):
            length0 = self.getOption('width')
            length1 = self.getOption('length')
            plane.setOptions(resolution=(1, n+1))
        else:
            length0 = self.getOption('length')
            length1 = self.getOption('width')
            plane.setOptions(resolution=(n+1, 1))

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
        plane.setOptions(origin=(pos[0], pos[1], 0),
                         point1=(p0[0], p0[1], 0),
                         point2=(p1[0], p1[1],0))

        # Set the colormap for the bar
        rng = self.getOption('cmap_range')
        step = (rng[1] - rng[0]) / float(n)

        data = vtk.vtkFloatArray()
        data.SetNumberOfTuples(n+1)
        for i in xrange(n+1):
            data.SetValue(i, rng[0] + i*step)
        plane.setOptions(data=data)

        # Setup the primary Axis
        axis0.options().update(self.getOption('primary'))
        location = self.__setAxisPosition(axis0, p0, p1, self.getOption('location'))

        # Setup the secondary Axis
        axis1.options().update(self.getOption('secondary'))
        self.__setAxisPosition(axis1, p0, p1, location)

    def onMouseMoveEvent(self, position):
        self.setOption('colorbar_origin', position)
        self.printOption('colorbar_origin')

    def _increment(self, increment, name, *args):
        """
        Helper for changing the width and length of the colorbar.
        """
        value = self.getOption(name) + increment
        if value < 1 and value > 0:
            self.printOption(name)
            self.setOption(name, value)

    def _incrementFont(self, increment, *args):
        """
        Helper for changing the font sizes.
        """

        def set_font_size(ax):
            """Helper for setting both the label and tile fonts."""
            fz_tick = ax.getVTKSource().GetLabelProperties().GetFontSize() + increment
            fz_title = ax.getVTKSource().GetTitleProperties().GetFontSize() + increment
            if fz_tick > 0:
                ax.setOption('tick_font_size', fz_tick)
                ax.printOption('tick_font_size')

            if fz_title > 0:
                ax.setOption('title_font_size', fz_title)
                ax.printOption('title_font_size')

        _, axis0, axis1 = self._sources
        set_font_size(axis0)
        set_font_size(axis1)


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
            axis.setOption('axis_point1', (pt1[0], pt0[1]))
            axis.setOption('axis_point2', (pt1[0], pt1[1]))
            secondary_location = 'right'

        elif location.lower() == 'right':
            axis.setOption('axis_point1', (pt0[0], pt0[1]))
            axis.setOption('axis_point2', (pt0[0], pt1[1]))
            secondary_location = 'left'

        elif location.lower() == 'top':
            axis.setOption('axis_point1', (pt1[0], pt1[1]))
            axis.setOption('axis_point2', (pt0[0], pt0[1]))
            secondary_location = 'bottom'

        elif location.lower() == 'bottom':
            axis.setOption('axis_point1', (pt1[0], pt0[1]))
            axis.setOption('axis_point2', (pt0[0], pt0[1]))
            secondary_location = 'top'
        return secondary_location
