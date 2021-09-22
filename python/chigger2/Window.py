#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html
import os
import sys
import logging
import vtk
from vtk.util.vtkAlgorithm import VTKPythonAlgorithmBase
from moosetools import mooseutils

from . import base
from . import annotations
from . import observers
from . import utils
from .Viewport import Viewport, Background

class Window(base.ChiggerAlgorithm):
    """
    Wrapper of vtkRenderWindow
    """
    __CHIGGER_CURRENT__ = None

    @staticmethod
    def validParams():
        opt = base.ChiggerAlgorithm.validParams()

        opt.add('size', default=(1920, 1080), vtype=int, size=2,
                doc="The size of the window, expects a list of two items")

        # TODO: Re-enable 2D mode for planar 3D data
        #opt.add('style', default='interactive', vtype=str,
        #        allow=('interactive', 'interactive2D'),
        #        doc="The interaction style ('interactive' enables 3D interaction, 'interactive2D' "\
        #            "disables out-of-plane interaction, and 'modal' disables all interaction.")

        opt.add('offscreen', default=False, vtype=bool,
                doc="Enable offscreen rendering.")

        #opt.add('chigger', False, "Places a chigger logo in the lower left corner.") #TODO
        opt.add('smoothing', default=False, vtype=bool,
                doc="Enable VTK render window smoothing options.")
        opt.add('multisamples', vtype=int,
                doc="Set the number of multi-samples.")
        opt.add('antialiasing', default=0, vtype=int,
                doc="Number of antialiasing frames to perform (set vtkRenderWindow::SetAAFrames).")

        # Background settings
        opt += utils.BackgroundParams.validParams()
        opt.setValue('background', dict(color=utils.Color(0,0,0)))

        # Writer settings
        opt.add('imagename', vtype=str, doc="The output image filename for the write command.")
        opt.add('transparent', default=False, vtype=bool,
                doc="When True images created will use a transparent background during image creation")

        # Observer
        opt.add('interactive', True, doc="Toggle indicating if the Window is interactive.")
        opt.add('observer', vtype=bool, default=True,
                doc="Create the default observer for command-line and mouse interaction.")

        return opt

    def __init__(self, **kwargs):
        Window.__CHIGGER_CURRENT__ = self
        base.ChiggerAlgorithm.__init__(self, nInputPorts=0, nOutputPorts=0, **kwargs)
        self.__vtkwindow = vtk.vtkRenderWindow()
        self.__vtkinteractor = self.__vtkwindow.MakeRenderWindowInteractor()
        self.__vtkinteractorstyle = None
        self.__viewports = list()
        self.__observer = None

        # Add the background
        # The interaction settings are tricky in VTK. The vtkInteractorStyle object calls
        # FindPokedRender, which always returns something regardless of the individual
        # vtkRenderer::SetInteractive settings. To get around this problem this background layer
        # is setup and serves as a fallback vtkRenderer object if all others are off.
        #
        # https://vtk.org/pipermail/vtkusers/2018-June/102030.html
        self.__background = Background(window=self)

        # TODO: Create "chigger" watermark
        """
        self.__watermark = annotations.ImageAnnotation(filename='chigger_white.png',
                                                       width=0.025,
                                                       horizontal_alignment='left',
                                                       vertical_alignment='bottom',
                                                       position=[0, 0])
        """

    def add(self, viewport):
        """(public)

        """
        #port = self.GetNumberOfInputPorts()
        #self.SetNumberOfInputPorts(port + 1)
        #self.SetInputConnection(port, viewport.GetOutputPort())

    #    #TODO: Type checking
        self.__viewports.append(viewport)

        renderer = viewport.getVTKRenderer()
        if not self.__vtkwindow.HasRenderer(renderer):
            self.__vtkwindow.AddRenderer(renderer)


    def updateInformation(self):
        base.ChiggerAlgorithm.updateInformation(self)
        for view in self.__viewports:
            view.updateInformation()

    def updateData(self):
        base.ChiggerAlgorithm.updateData(self)
        for view in self.__viewports:
            view.updateData()
            #view.getVTKRenderer().ResetCamera()

    def viewports(self):
        """(public)
        Access to the Viewport objects.
        """
        return self.__viewports

    def getVTKInteractor(self):
        """
        Return the vtkInteractor object.
        """
        return self.__vtkinteractor

    def getVTKInteractorStyle(self):
        """
        Return the vtkInteractor object.
        """
        return self.__vtkinteractorstyle

    def getVTKWindow(self):
        """
        Return the vtkRenderWindow object.
        """
        return self.__vtkwindow

    def start(self):
        """
        Begin the interactive VTK session.
        """
        self.debug("start")
        self.updateInformation()
        self.updateData()
        self.__vtkwindow.Render()
        if self.__vtkinteractorstyle:
            self.__vtkinteractor.Initialize()
            self.__vtkinteractor.Start()

    def terminate(self):
        self.__vtkwindow.Finalize()
        if self.__vtkinteractor is not None:
            self.__vtkinteractor.TerminateApp()
        del self.__vtkwindow, self.__vtkinteractor
        self.__vtkwindow = None



    #def __del__(self):
    #    base.ChiggerAlgorithm.__del__(self)
    #    for view in self.__viewports:
    #        del view
    #    self.__viewports = None

    def _onRequestInformation(self, *args):
        base.ChiggerAlgorithm._onRequestInformation(self, *args)

        # Viewport Layers
        n = self.__vtkwindow.GetNumberOfLayers()
        for view in self.__viewports:
            n = max(n, view.getParam('layer') + 1)
        self.__vtkwindow.SetNumberOfLayers(n)

        # Background Setting
        self.__background.getParam('background').update(self.getParam('background'))
        self.__background._vtkrenderer.SetBackgroundAlpha(1)

        # Auto Background adjustments
        children = [source for viewport in self.__viewports for source in viewport.sources()]
        utils.auto_adjust_color(self, children)

        # Interactive/Observer settings
        if self.getParam('offscreen'):
            self.__vtkwindow.OffScreenRenderingOn()
        #elif not self.getParam('interactive'):
        #    self.__vtkinteractorstyle = vtk.vtkInteractorStyleUser()
        else:
            # TODO: Restore 2D option for 3D objects in plane
            if self.__vtkinteractorstyle is None:
                self.__vtkinteractorstyle = vtk.vtkInteractorStyleJoystickCamera()
                self.__vtkinteractor.SetInteractorStyle(self.__vtkinteractorstyle)
                self.__vtkinteractor.RemoveObservers(vtk.vtkCommand.CharEvent)


            # TODO: Create  object in constructor, just setup things here based on 'style'
            #if self.__vtkinteractor is None:
            #    self.__vtkinteractor = self.__vtkwindow.MakeRenderWindowInteractor()
                #self.__vtkinteractor = ChiggerInteractor(self)
                #self.__vtkinteractor.SetRenderWindow(self.__vtkwindow)

            # TODO: This is handled by MainWindowObserver
            #if style == 'interactive':
            #    self.__vtkinteractorstyle = vtk.vtkInteractorStyleJoystickCamera()
            #elif style == 'interactive2d':
            #    self.__vtkinteractorstyle = vtk.vtkInteractorStyleImage()
            #elif style == 'modal':
            #    self.__vtkinteractorstyle = vtk.vtkInteractorStyleUser()



            # Add/remove the default MainWindowObserver
            if self.getParam('observer') and (self.__observer is None):
                self.__observer = observers.MainWindowObserver(window=self)
            elif (not self.getParam('observer')) and (self.__observer is not None):
                del self.__observer
                self.__observer = None

        # vtkRenderWindow Settings
        self.assignParam('offscreen', self.__vtkwindow.SetOffScreenRendering)
        self.assignParam('smoothing', self.__vtkwindow.SetLineSmoothing)
        self.assignParam('smoothing', self.__vtkwindow.SetPolygonSmoothing)
        self.assignParam('smoothing', self.__vtkwindow.SetPointSmoothing)
        self.assignParam('multisamples', self.__vtkwindow.SetMultiSamples)
        self.assignParam('size', self.__vtkwindow.SetSize)



    def _onRequestData(self, *args):
        base.ChiggerAlgorithm._onRequestData(self, *args)
        self.__vtkwindow.Render()

    def render(self):
        if self.__vtkwindow is not None:
            self.updateInformation()
            self.updateData()
            self.__vtkwindow.Render()

    def resetCamera(self):
        """
        Resets all the cameras.

        Generally, this is not needed but in some cases when testing the camera needs to be reset
        for the image to look correct.
        """
        for view in self.__viewports:
            view.getVTKRenderer().ResetCamera()

    def resetClippingRange(self):
        """
        Resets all the clipping range for open cameras.
        """
        for view in self.__viewports:
            view.getVTKRenderer().ResetCameraClippingRange()

    def write(self, **kwargs):
        """
        Writes the VTKWindow to an image.
        """
        self.debug('write')
        self.setParams(**kwargs)
        self.updateInformation()
        self.updateData()

        filename = self.getParam('imagename')

        # Allowed extensions and the associated readers
        writers = dict()
        writers['.png'] = vtk.vtkPNGWriter
        writers['.ps'] = vtk.vtkPostScriptWriter
        writers['.tiff'] = vtk.vtkTIFFWriter
        writers['.bmp'] = vtk.vtkBMPWriter
        writers['.jpg'] = vtk.vtkJPEGWriter

        # Extract the extension
        _, ext = os.path.splitext(filename)
        if ext not in writers:
            msg = "The filename must end with one of the following extensions: {}."
            self.error(msg, ', '.join(writers.keys()))
            return
        # Check that the directory exists
        dirname = os.path.dirname(filename)
        if (len(dirname) > 0) and (not os.path.isdir(dirname)):
            self.error("The directory does not exist: {}", dirname)
            return

        # Build a filter for writing an image
        window_filter = vtk.vtkWindowToImageFilter()
        window_filter.SetInput(self.__vtkwindow)

        # Allow the background to be transparent
        if self.getParam('transparent'):
            window_filter.SetInputBufferTypeToRGBA()

        self.__vtkwindow.Render()
        window_filter.Update()

        # Write it
        writer = writers[ext]()
        writer.SetFileName(filename)
        writer.SetInputData(window_filter.GetOutput())
        writer.Write()
