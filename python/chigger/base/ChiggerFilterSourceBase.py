#pylint: disable=missing-docstring
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import copy
import vtk
import mooseutils
from .ChiggerSourceBase import ChiggerSourceBase

class ChiggerFilterSourceBase(ChiggerSourceBase):
    """
    A base class for creating "source" objects (in VTK something that needs an vtkActor) that
    require additional input into the mapper and are capable of accepting filters.

    This class adds two main items:

    1. A getSource method is provided, this method should provide a VTK object that will be connect
       to the mapper or chain of filters (see 2).
    2. Defines a method for adding filters and controlling the types and order in which they are
       applied, see ExodusSource for example.

    Inputs:
        vtkactor_type: The VTK actor type to build, must be an instance of VTKACTOR_TYPE
        vtkmapper_type: The VTK mapper type to build, must be an instance of VTKMAPPER_TYPE
        **kwargs: The key, value options for this object.
    """
    # The base class actor/mapper that this object to which ownership is restricted
    VTKACTOR_TYPE = vtk.vtkProp
    VTKMAPPER_TYPE = vtk.vtkAbstractMapper

    # The list of filter types allowed, in the order they should be connected
    FILTER_TYPES = []

    @staticmethod
    def getOptions():
        opt = ChiggerSourceBase.getOptions()
        opt.add('filters', [], "A list of Filter objects to apply to this mapper.")
        return opt

    def __init__(self, *args, **kwargs):
        super(ChiggerFilterSourceBase, self).__init__(*args, **kwargs)
        self._filters = []
        self._required_filters = []

    def getVTKSource(self):
        """
        Return the "source" vtk object. (abstract)
        classes must override this method. The VTK object returned from this function will
        be connected to the first filter, if then exist, or the vtkAbstractMapper object. See the
        'update' method for this class for how the connections are made.
        """
        raise mooseutils.MooseException('The {}."getSource()" method must be overridden by your '
                                        'mapper object and return the source vtk object to connect '
                                        'to the filers and mapper.'.format(self.__class__.__name__))

    def getFilters(self):
        """
        Return the list of filter objects.
        """
        return self._filters

    def needsUpdate(self):
        """
        Return True if the object needs to be updated.
        """
        changed = [super(ChiggerFilterSourceBase, self).needsUpdate()]
        for f in self._filters:
            changed.append(f.needsUpdate())
        return any(changed)

    def update(self, **kwargs):
        """
        Updates the object by connecting the VTK objects. (override)

        Inputs:
            see ChiggerSourceBase
        """
        super(ChiggerFilterSourceBase, self).update(**kwargs)

        self.__connectFilters()

        # Initialize and update filters
        for f in self._filters:
            if f.needsInitialize():
                f.initializeFilter(self)
            if f.needsUpdate():
                f.update()

    def __connectFilters(self):
        """
        Helper function for connecting filter to vtkMapper object.
        """

        def debug(src, fltr):
            """
            Inline function for debug messages.
            """
            mooseutils.mooseDebug('{} --> {}'.format(type(src).__name__, type(fltr).__name__),
                                  color='GREEN')

        # Create a list of filters to apply to the VTK pipeline, this is done by
        # combining the required filters with the 'filters' options. This combined list
        # is then sorted based on the list provided in FILTER_TYPES.
        filters = []
        filters_in = copy.copy(self._required_filters) # shallow copy (don't modify require list)
        if self.isOptionValid('filters'):
            filters_in += self.getOption('filters')

        for f in filters_in:
            for i, order in enumerate(self.FILTER_TYPES):
                if isinstance(f, order):
                    filters.append((f, i))
        self._filters = [f[0] for f in sorted(filters, key=lambda x: x[1])]

        # Connect the filters, if any exist
        if self._filters:
            debug(self.getVTKSource(), self._filters[0].getVTKFilter())
            self._filters[0].getVTKFilter().SetInputConnection(self.getVTKSource().GetOutputPort())

            for i in range(1, len(self._filters)):
                debug(self._filters[i-1].getVTKFilter(), self._filters[i].getVTKFilter())
                f = self._filters[i-1].getVTKFilter().GetOutputPort()
                self._filters[i].getVTKFilter().SetInputConnection(f)

            if self._vtkmapper:
                debug(self._filters[-1].getVTKFilter(), self._vtkmapper)
                self._vtkmapper.SetInputConnection(self._filters[-1].getVTKFilter().GetOutputPort())

        elif self._vtkmapper:
            debug(self.getVTKSource(), self._vtkmapper)
            self._vtkmapper.SetInputConnection(self.getVTKSource().GetOutputPort())
