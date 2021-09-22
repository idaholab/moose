#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html
import logging
import weakref
import collections
import vtk
from moosetools import mooseutils

from .. import utils
from .ChiggerAlgorithm import ChiggerAlgorithm

@mooseutils.addProperty('filter_type', required=True)
@mooseutils.addProperty('active', ptype=bool, default=True)
class FilterInfo(mooseutils.AutoPropertyMixin):
    """
    Storage for Filter object information.

    This is a stand-alone class to allow for the 'active' property to be altered.
    """
    pass


class ChiggerSourceBase(utils.KeyBindingMixin, ChiggerAlgorithm):

    # The type of vtkProp to create (this should be defined in child class)
    VTKACTORTYPE = None

    # The type of vtkAbstractMapper to create (this should be defined in child class)
    VTKMAPPERTYPE = None

    @classmethod
    def validParams(cls):
        opt = ChiggerAlgorithm.validParams()
        opt += utils.KeyBindingMixin.validParams()

        opt.add('pickable', default=True, vtype=bool,
                doc="Indicates if the source can be selected via the MainWindowObserver")

        opt.add('interactive', default=True, vtype=bool,
                doc="Marks the source active for the MainWindowObserver")
        opt.add('highlight', default=False, vtype=bool,
                doc="Highlight the object.")

        """
        opt.add('color', vtype=(int, float), size=3,
                doc="The color of the source")
        opt.add('linewidth', vtype=(int, float),
                doc="The linewidth of the source")
        """

        opt.add('viewport', default=utils.get_current_viewport(), required=True,
                doc='The chigger.Viewport object that this source is to be associated')
        return opt

    @staticmethod
    def validKeyBindings():
        bindings = utils.KeyBindingMixin.validKeyBindings()
        bindings.add('o', ChiggerSourceBase._setOpacity, args=(0.02,),
                     desc="Increase opacity by 2%.")
        bindings.add('o', ChiggerSourceBase._setOpacity, args=(-0.02,), shift=True,
                     desc="Decrease opacity by 2%.")
        return bindings

    def __init__(self, **kwargs):

        # Storage for the available filters for this object, this needs to be before the base
        # class __init__ because the setParams command of this class attempts to apply options to
        # the filters.
        self.__filter_info = list()

        utils.KeyBindingMixin.__init__(self)
        ChiggerAlgorithm.__init__(self, **kwargs)

        # Storage for the outline object created with 'higlight' option
        self._outline = None

        # Create mapper
        self._vtkmapper = self.VTKMAPPERTYPE() if self.VTKMAPPERTYPE else None
        if (self._vtkmapper is not None) and not isinstance(self._vtkmapper, vtk.vtkAbstractMapper):
            msg = 'The supplied mapper is a {} but must be a vtk.vtkAbstractMapper type.'
            raise mooseutils.MooseException(msg.format(type(self._vtkmapper).__name__))

        # Create the actor
        self._vtkactor = self.VTKACTORTYPE() if self.VTKACTORTYPE else None
        if (self._vtkactor is not None) and not isinstance(self._vtkactor, vtk.vtkProp):
            msg = 'The supplied actor is a {} but must be a vtk.vtkProp type.'
            raise mooseutils.MooseException(msg.format(type(self._vtkactor).__name__))

        # Connect the mapper and actor and add the actor to the renderer
        if self._vtkmapper is not None:
            self._vtkactor.SetMapper(self._vtkmapper)

        # Storage for the filter object instances
        self._filters = collections.OrderedDict()

        # Add this ChiggerSource object to the viewport
        self._viewport.add(self)

    @property
    def _viewport(self):
        """Property so that self._viewport acts like the actual Viewport object."""
        return self.getParam('viewport')

    def _addFilter(self, filter_type, active=False):
        self.__filter_info.append(FilterInfo(filter_type=filter_type, active=active))

        fname = filter_type.FILTERNAME
        self._parameters.add(fname, filter_type.validParams(),
                          doc="Params for the '{}' filter.".format(fname))

    def remove(self):
        self._viewport.remove(self)

    def getVTKActor(self):
        """
        Return the constructed vtk actor object. (public)

        Returns:
            An object derived from vtk.vtkProp.
        """
        return self._vtkactor

    def getVTKMapper(self):
        """
        Return the constructed vtk mapper object. (public)

        Returns:
            An object derived from vtk.vtkAbstractMapper
        """
        return self._vtkmapper

    def getFilters(self):
        return self._filters

    def setParams(self, *args, **kwargs):
        ChiggerAlgorithm.setParams(self, *args, **kwargs)

        for finfo in self.__filter_info:
            if finfo.filter_type.FILTERNAME in args:
                finfo.active = True
                self.__ACTIVE_FILTERS__.add(filter_type.FILTERNAME)

    def _onRequestInformation(self, *args):
        ChiggerAlgorithm._onRequestInformation(self, *args)

        # Apply Actor options (must exist in both vtkProperty and vtkProperty2D
        #self.assignParam('linewidth', self._vtkactor.GetProperty().SetLineWidth)
        #self.assignParam('color', self._vtkactor.GetProperty().SetColor)

        # Connect the filters
        self.__connectFilters()

        # Create/Remove highlight
        self._highlight()

    def __connectFilters(self):
        self.debug('__connectFilters')

        for finfo in self.__filter_info:
            fname = finfo.filter_type.FILTERNAME
            if (finfo.active) and (fname not in self._filters):
                self._filters[fname] = finfo.filter_type()
                self.Modified()
            elif (not finfo.active) and (fname in self._filters):
                self._filters.pop(fname)
                self.Modified()

        base_obj = self
        for fname, filter_obj in self._filters.items():
            self.debug('{} --> {}'.format(filter_obj.name(), base_obj.name()))
            filter_obj.SetInputConnection(0, base_obj.GetOutputPort(0))
            base_obj = filter_obj

        # Connect mapper/filters into the pipeline
        if (self._vtkmapper is not None) and (base_obj.GetNumberOfOutputPorts()):
            self.debug('{} --> {}'.format(self._vtkmapper.GetClassName(), base_obj.name()))
            self._vtkmapper.SetInputConnection(0, base_obj.GetOutputPort(0))

    def _highlight(self):
        """
        Apply/remove highlight based on 'highlight' option.

        This is a stand-alone function to allow for object to customize the behavior if needed. For
        example the TextBase object overrides this method. In truth the text object was the
        motivating factor for creating this function and handling the highlighting in this way. Many
        days have been lost trying to create a generic method for highlighting an object and this
        was the best I could do. Perhaps a VTK ninja could do something better.

        Dear Future Andrew,
        Don't change this basic design, you spent way too much time getting it to this. Leave it.
        - Andrew (9.18.2020)
        """
        if self.getParam('highlight') and (self._outline is None):
            from .. import geometric # avoid cyclic import
            is_3D = isinstance(self.getVTKActor(), vtk.vtkActor)
            obj_type = geometric.Highlight if is_3D else geometric.Highlight2D
            offset = 0.05 if is_3D else 0.02
            self._outline = obj_type(viewport=self._viewport, source=self, pickable=False,
                                     linewidth=3, color=utils.Color(1,1,0))
        elif (not self.getParam('highlight')) and (self._outline is not None):
            self._outline.remove()
            del self._outline
            self._outline = None

    def _setOpacity(self, delta):
        value = self.getParam('opacity') + delta
        value = 0 if value < 0 else 1 if value > 1 else value
        self.setParam('opacity', value)
        self.printOption('opacity')

    def __del__(self):
        """
        Delete the actor and mapper, this is needed to avoid a seg fault in VTK
        """
        self._vtkactor = None
        self._vtkmapper = None

class ChiggerSource(ChiggerSourceBase):

    VTKACTORTYPE = vtk.vtkActor

    @classmethod
    def validParams(cls):
        opt = ChiggerSourceBase.validParams()

        opt.add('representation', default='surface', allow=('surface', 'wireframe', 'points'),
                doc="View volume representation.")

        #opt.add('edges', default=False, vtype=bool,
        #        doc="Enable edges on the rendered object.")
        #opt.add('edgecolor', default=(0.5,)*3, array=True, size=3, vtype=(int, float),
        #        doc="The color of the edges, 'edges=True' must be set.")
        #opt.add('edgewidth', default=1, vtype=(float, int),
        #        doc="The width of the edges, 'edges=True' must be set.")

        opt.add('lines_as_tubes', default=False, vtype=bool,
                doc="Toggle rendering 1D lines as tubes.")

        return opt

    def _onRequestInformation(self, *args):
        ChiggerSourceBase._onRequestInformation(self, *args)

        rep = self.getParam('representation')
        if rep == 'surface':
            self._vtkactor.GetProperty().SetRepresentationToSurface()
        elif rep == 'wireframe':
            self._vtkactor.GetProperty().SetRepresentationToWireframe()
        elif rep == 'points':
            self._vtkactor.GetProperty().SetRepresentationToPoints()

        #self.assignParam('edges', self._vtkactor.GetProperty().SetEdgeVisibility)
        #self.assignParam('edgecolor', self._vtkactor.GetProperty().SetEdgeColor)
        #self.assignParam('edgewidth', self._vtkactor.GetProperty().SetLineWidth)

        self.assignParam('lines_as_tubes', self._vtkactor.GetProperty().SetRenderLinesAsTubes)

class ChiggerSource2D(ChiggerSourceBase):
    VTKACTORTYPE = vtk.vtkActor2D

    @classmethod
    def validParams(cls):
        opt = ChiggerSourceBase.validParams()
        return opt
