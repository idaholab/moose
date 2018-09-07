#pylint: disable=missing-docstring
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html
import collections
import textwrap
import vtk
import mooseutils

import chigger
from .. import utils
from ChiggerObject import ChiggerObject

class ChiggerResultBase(ChiggerObject, utils.KeyBindingMixin):
    """
    Base class for objects to be displayed with a single vtkRenderer object.

    Any object or set of objects that require a single vtkRenderer object should inherit from this
    and all settings for the vtkRender object should be placed in this class.

    If you are creating a new type of "result" object (i.e., something with a vtkRenderer) you will
    likely want to derive from one of the child classes of ChiggerResultBase, such as ChiggerResult.

    Inputs:
        see ChiggerObject
    """
    @staticmethod
    def validOptions():
        opt = ChiggerObject.validOptions()
        opt.add('interactive', default=True,
                doc="Control if the object may be selected with key bindings.")
        opt.add('layer', default=1, vtype=int,
                doc="The VTK layer within the render window.")
        opt.add('viewport', default=(0., 0., 1., 1.), vtype=float, size=4,
                doc="A list given the viewport coordinates [x_min, y_min, x_max, y_max], in " \
                    "relative position to the entire window (0 to 1).")
        opt.add('camera', None, vtype=vtk.vtkCamera,
                doc="The VTK camera to utilize for viewing the results.")
        opt.add('highlight_active', default=True, vtype=bool,
                doc="When the result is activate enable/disable the 'highlighting'.")
        return opt

    @staticmethod
    def validKeyBindings():
        bindings = utils.KeyBindingMixin.validKeyBindings()
        bindings.add('c', ChiggerResultBase._printCamera,
                     desc="Display the camera settings for this object.")
        return bindings

    def __init__(self, renderer=None, **kwargs):
        super(ChiggerResultBase, self).__init__(**kwargs)
        self._vtkrenderer = renderer if renderer != None else vtk.vtkRenderer()
        self._vtkrenderer.SetInteractive(False)
        self._render_window = None
        self.__highlight = None

    def getRenderWindow(self):
        """
        Return the chigger.RenderWindow object.
        """
        return self._render_window

    def getVTKRenderer(self):
        """
        Return the vtkRenderer object. (public)

        Generally, this should not be used. This method if mainly for the RenderWindow object to
        populate the list of renderers that it will be displaying.
        """
        return self._vtkrenderer

    def update(self, **kwargs):
        """
        Update the vtkRenderer settings. (override)

        Inputs:
            see ChiggerObject
        """
        super(ChiggerResultBase, self).update(**kwargs)

        # Render layer
        if self.isOptionValid('layer'):
            self._vtkrenderer.SetLayer(self.applyOption('layer'))

        # Viewport
        if self.isOptionValid('viewport'):
            self._vtkrenderer.SetViewport(self.applyOption('viewport'))

        # Camera
        if self.isOptionValid('camera'):
            self._vtkrenderer.SetActiveCamera(self.applyOption('camera'))

    def getBounds(self):
        """
        Return the bounding box of the results.

        By default this returns the bounding box of the viewport.
        """
        origin = self.getVTKRenderer().GetOrigin()
        size = self.getVTKRenderer().GetSize()
        return [origin[0], origin[0] + size[0], origin[1], origin[1] + size[1], 0, 0]

    def _printCamera(self, *args):
        """Keybinding callback."""
        print '\n'.join(utils.print_camera(self._vtkrenderer.GetActiveCamera()))

    def init(self, window):
        """
        Initialize the object. This an internal method, DO NOT USE!

        When a ChiggerResultBase object is ADDED to the RenderWindow this is called automatically.
        """
        self._render_window = window

    def deinit(self):
        """
        De-Initialize the object. This an internal method, DO NOT USE!

        When a ChiggerResultBase object is REMOVED to the RenderWindow this is called automatically.
        """
        pass

    def setActive(self, value):
        """
        Activate method. This is an internal method, DO NOT USE!

        Use RenderWindow.setActive() to activate/deactivate result objects.
        """
        self._vtkrenderer.SetInteractive(value)
        if value and self.getOption('highlight_active'):
            if self.__highlight is None:
                self.__highlight = chigger.geometric.OutlineResult(self,
                                                                   interactive=False,
                                                                   color=(1,0,0),
                                                                   line_width=5)
            self.getRenderWindow().append(self.__highlight)

        elif self.__highlight is not None:
            self.getRenderWindow().remove(self.__highlight)
