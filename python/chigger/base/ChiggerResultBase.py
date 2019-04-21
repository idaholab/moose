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
from .ChiggerObject import ChiggerObject

class ChiggerResultBase(ChiggerObject):
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
    def getOptions():
        opt = ChiggerObject.getOptions()
        opt.add('layer', 1, "The VTK layer within the render window.", vtype=int)
        opt.add('viewport', [0, 0, 1, 1], "A list given the viewport coordinates [x_min, y_min, "
                                          "x_max, y_max], in relative position to the entire "
                                          "window (0 to 1).", vtype=list)
        opt.add('background', [0, 0, 0], "The background color, only applied when the 'layer' "
                                         "option is zero. A background result is automatically "
                                         "added when chigger.RenderWindow is utilized.")
        opt.add('background2', None, "The second background color, when supplied this creates a "
                                     "gradient background, only applied when the 'layer' option is "
                                     "zero. A background result is automatically added when "
                                     "chigger.RenderWindow is utilized.", vtype=list)
        opt.add('gradient_background', False, "Enable/disable the use of a gradient background.")
        opt.add('camera', "The VTK camera to utilize for viewing the results.", vtype=vtk.vtkCamera)
        opt.add('light', None, "Add a headlight with the supplied intensity.", vtype=float)
        return opt

    def __init__(self, renderer=None, **kwargs):
        super(ChiggerResultBase, self).__init__(**kwargs)
        self._vtkrenderer = renderer if renderer != None else vtk.vtkRenderer()
        self._vtklight = vtk.vtkLight()
        self._vtklight.SetLightTypeToHeadlight()

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
            self._vtkrenderer.SetLayer(self.getOption('layer'))

        # Viewport
        if self.isOptionValid('viewport'):
            self._vtkrenderer.SetViewport(self.getOption('viewport'))

        # Background (only gets applied if layer=0)
        self._vtkrenderer.SetBackground(self.getOption('background'))
        if self.isOptionValid('background2'):
            self._vtkrenderer.SetBackground2(self.getOption('background2'))

        if self.isOptionValid('gradient_background'):
            self._vtkrenderer.SetGradientBackground(self.getOption('gradient_background'))

        # Camera
        if self.isOptionValid('camera'):
            self._vtkrenderer.SetActiveCamera(self.getOption('camera'))

        # Headlight
        if self.isOptionValid('light'):
            self._vtklight.SetIntensity(1.5)
            self._vtkrenderer.AddLight(self._vtklight)
