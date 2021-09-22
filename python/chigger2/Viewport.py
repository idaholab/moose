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
import vtk
from vtk.util.vtkAlgorithm import VTKPythonAlgorithmBase
from moosetools import mooseutils
from . import utils
from . import base
from . import geometric

class Viewport(utils.KeyBindingMixin, base.ChiggerAlgorithm):
    """
    Viewport object define a region within the window for displaying source objects, it
    is a wrapper around the VTK vtkRenderer object.
    """

    # Used by utils.get_current_viewport for automatically adding source objects to the current
    # Viewport, in similar fashion that matplotlib operates.
    __CHIGGER_CURRENT__ = None

    @classmethod
    def validParams(cls):
        opt = base.ChiggerAlgorithm.validParams()
        opt += utils.KeyBindingMixin.validParams()
        opt += utils.BackgroundParams.validParams()

        opt.add('light', vtype=(int, float),
               doc="Add a headlight with the given intensity to the viewport.")
        opt.add('layer', default=1, vtype=int,
                doc="The layer within the render window.")

        opt.add('xmin', default=0, vtype=(int, float),
                verify=(lambda v: v>=0 and v<=1, "Value must be in range [0,1]"))
        opt.add('xmax', default=1, vtype=(int, float),
                verify=(lambda v: v>=0 and v<=1, "Value must be in range [0,1]"))
        opt.add('ymin', default=0, vtype=(int, float),
                verify=(lambda v: v>=0 and v<=1, "Value must be in range [0,1]"))
        opt.add('ymax', default=1, vtype=(int, float),
                verify=(lambda v: v>=0 and v<=1, "Value must be in range [0,1]"))

        opt.add('camera', None, vtype=vtk.vtkCamera,
                doc="The VTK camera to utilize for viewing the results.")

        opt.add('interactive', vtype=bool, doc="Toggle indicating if the Viewport is interactive.")
        opt.add('highlight', vtype=bool, default=False,
                doc="Toggle highlighting of the viewport boundary.")

        opt.add('window', default=utils.get_current_window(), required=True,
                doc='The chigger.Window object that this Viewport is to be associated')

        return opt

    @staticmethod
    def validKeyBindings():

        bindings = utils.KeyBindingMixin.validKeyBindings()

        bindings.add('right', Viewport._setViewport, args=(0, 0.025),
                     desc="Increase the viewport x-min value.")
        bindings.add('left', Viewport._setViewport, args=(0, -0.025),
                     desc="Increase the viewport x-min value.")

        bindings.add('right', Viewport._setViewport, args=(2, 0.025), shift=True,
                     desc="Decrease the viewport x-max value.")
        bindings.add('left', Viewport._setViewport, args=(2, -0.025), shift=True,
                     desc="Decrease the viewport x-max value")

        bindings.add('up', Viewport._setViewport, args=(1, 0.025),
                     desc="Increase the viewport y-min value.")
        bindings.add('down', Viewport._setViewport, args=(1, -0.025),
                     desc="Increase the viewport y-min value.")

        bindings.add('up', Viewport._setViewport, args=(3, 0.025), shift=True,
                     desc="Decrease the viewport y-max value.")
        bindings.add('down', Viewport._setViewport, args=(3, -0.025), shift=True,
                     desc="Decrease the viewport y-max value")

        return bindings

    def __init__(self, **kwargs):
        Viewport.__CHIGGER_CURRENT__ = self
        utils.KeyBindingMixin.__init__(self)
        base.ChiggerAlgorithm.__init__(self, nInputPorts=0, nOutputPorts=0, **kwargs)

        # Initialize class members
        self._vtkrenderer = vtk.vtkRenderer()
        self.__sources = list()
        self.__outline = None

        # Add the viewport to the window
        self._window.add(self)

    @property
    def _window(self):
        """Property so that self._window acts like the actual window object."""
        return self.getParam('window')

    def add(self, arg):
        actor = arg.getVTKActor()
        if actor is not None:
            self._vtkrenderer.AddActor(actor)
        self.__sources.append(arg)

    def remove(self, arg):
        """TODO: change remove to have objects self remove"""

        if arg in self.__sources:
            self.__sources.remove(arg)
            if arg.getVTKActor() is not None:
                self._vtkrenderer.RemoveActor(arg.getVTKActor())

    def updateInformation(self):
        """TODO...explain this and why making this part of the pipeline is not ideal"""
        base.ChiggerAlgorithm.updateInformation(self)
        for source in self.__sources:
            source.updateInformation()

    def updateData(self):
        base.ChiggerAlgorithm.updateData(self)
        for source in self.__sources:
            source.updateData()

    def _onRequestInformation(self, *args):
        base.ChiggerAlgorithm._onRequestInformation(self, *args)

        vp = [self.getParam('xmin'), self.getParam('ymin'), self.getParam('xmax'), self.getParam('ymax')]
        self._vtkrenderer.SetViewport(vp)

        self.assignParam('layer', self._vtkrenderer.SetLayer)
        self.assignParam('interactive', self._vtkrenderer.SetInteractive)

        # Add/Remove highlight
        if self.getParam('highlight') and (self.__outline is None):
            self.__outline = geometric.Outline2D(viewport=self, xmax=0.9999, ymax=0.9999,
                                                 color=utils.Color(1,1,0), linewidth=3, pickable=False)
        elif (not self.getParam('highlight')) and (self.__outline is not None):
            self.__outline.remove()
            del self.__outline
            self.__outline = None

        # Background Setting
        utils.BackgroundParams.applyParams(self.getVTKRenderer(),
                                             self.getParam('background'))

        if (self.isParamValid('background', 'color') or self.isParamValid('background', 'color2')) and self.getParam('layer') != 0:
            self.warning("The 'layer' option is set to {} and 'background_color' and/or 'background_color2' " \
                         "option(s) were provided, the color will not change unless the 'layer' is 0",
                         self._vtkrenderer.GetLayer())

        # Auto adjust background color
        if self._vtkrenderer.GetLayer() == 0:
            utils.auto_adjust_color(self, self.__sources)

    def getVTKRenderer(self):
        """Return the vtk.vtkRenderer object."""
        return self._vtkrenderer

    def sources(self):
        return self.__sources

    def increaseXmin(self, *args):
        self._setViewport(0, 0.05)

    def decreaseXmin(self, *args):
        self._setViewport(0, -0.05)

    def increaseXmax(self, *args):
        self._setViewport(2, 0.05)

    def decreaseXmax(self, *args):
        self._setViewport(2, -0.05)

    def _setViewport(self, index, increment):
        names = ['xmin', 'ymin', 'xmax', 'ymax']
        #x_min = self.getParam('xmin')
        #x_max = self.getParam('xmax')
        #y_min = self.getParam('ymin')
        #y_max = self.getParam('ymax')
        c = [self.getParam(n) for n in names]
        c[index] += increment

        xmin = round(c[0], 3) if (c[0] >= 0 and c[0] < c[2]) else xmin
        xmax = round(c[2], 3) if (c[2] <= 1 and c[0] < c[2]) else xmax

        ymin = round(c[1], 3) if (c[1] >= 0 and c[1] < c[3]) else ymin
        ymax = round(c[3], 3) if (c[3] <= 1 and c[1] < c[3]) else ymax

        self.setParams(xmin=xmin, xmax=xmax, ymin=ymin, ymax=ymax)
        self.updateInformation()
        self.printOption(['xmin', 'ymin', 'ymin', 'ymax'][index])

class Background(Viewport):
    @classmethod
    def validParams(cls):
        opt = Viewport.validParams()
        opt.setValue('name', '__ChiggerWindowBackground__')
        opt.setValue('highlight', False)
        opt.setValue('interactive', True)
        opt.setValue('layer', 0)
        opt.setValue('background', 'color', utils.Color(0., 0., 0.))
        return opt
