#pylint: disable=missing-docstring
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import os
import vtk

import mooseutils
from . import base
from . import annotations
from . import observers
from . import misc

VTK_MAJOR_VERSION = vtk.vtkVersion.GetVTKMajorVersion()

class RenderWindow(base.ChiggerObject):
    """
    Wrapper of VTK RenderWindow for use with ChiggerResultBase objects.
    """
    RESULT_TYPE = base.ChiggerResultBase

    @staticmethod
    def getOptions():
        opt = base.ChiggerObject.getOptions()

        opt.add('size', [960, 540], "The size of the window, expects a list of two items",
                vtype=list)
        opt.add('style', 'interactive', "The interaction style.",
                allow=['interactive', 'modal', 'interactive2D'])
        opt.add('test', False, "When True the interaction is disabled and the window closes "
                               "immediately after rendering.")
        opt.add('motion_factor', "Control the interaction motion rate. "
                                 "(calls vtkInteractorStyle::SetMotionFactor)",
                vtype=float)
        opt.add('offscreen', False, "Enable offscreen rendering.")
        opt.add('chigger', False, "Places a chigger logo in the lower left corner.")
        opt.add('smoothing', False, "Enable VTK render window smoothing options.")
        opt.add('antialiasing', 0, "Number of antialiasing frames to perform "
                                   "(set vtkRenderWindow::SetMultiSamples).", vtype=int)

        # Observers
        opt.add('observers', [], "A list of ChiggerObserver objects, once added they are not " \
                                 "removed. Hence, changing the observers in this list will not " \
                                 "remove existing objects.")

        # Background settings
        background = misc.ChiggerBackground.getOptions()
        background.pop('layer')
        background.pop('camera')
        background.pop('viewport')
        opt += background
        return opt

    def __init__(self, *args, **kwargs):

        self.__vtkwindow = kwargs.pop('vtkwindow', vtk.vtkRenderWindow())
        self.__vtkinteractor = kwargs.pop('vtkinteractor', None)

        super(RenderWindow, self).__init__(**kwargs)

        self._results = [misc.ChiggerBackground()]
        self._groups = []
        self.__active = None

        self.__watermark = annotations.ImageAnnotation(filename='chigger_white.png',
                                                       width=0.025,
                                                       horizontal_alignment='left',
                                                       vertical_alignment='bottom',
                                                       position=[0, 0])
        # Store the supplied result objects
        self.append(*args)
        if kwargs.pop('chigger', False):
            self.append(self.__watermark)

    def __contains__(self, item):
        """
        'in' checks for result
        """
        return item in self._results

    def getVTKInteractor(self):
        """
        Return the vtkInteractor object.
        """
        return self.__vtkinteractor

    def getVTKWindow(self):
        """
        Return the vtkRenderWindow object.
        """
        return self.__vtkwindow

    def append(self, *args):
        """
        Append result object(s) to the window.
        """
        self.setNeedsUpdate(True)
        for result in args:
            mooseutils.mooseDebug('RenderWindow.append {}'.format(type(result).__name__))
            if isinstance(result, base.ResultGroup):
                self.append(*result.getResults())
                self._groups.append(result)
            elif not isinstance(result, self.RESULT_TYPE):
                n = result.__class__.__name__
                t = self.RESULT_TYPE.__name__
                msg = 'The supplied result type of {} must be of type {}.'.format(n, t)
                raise mooseutils.MooseException(msg)
            self._results.append(result)

    def remove(self, *args):
        """
        Remove result object(s) from the window.
        """
        self.setNeedsUpdate(True)
        for result in args:
            if self.__vtkwindow.HasRenderer(result.getVTKRenderer()):
                self.__vtkwindow.RemoveRenderer(result.getVTKRenderer())
            if result in self._results:
                self._results.remove(result)

        # Reset active if it was removed
        if self.__active and (self.__active not in self._results):
            self.__active = None

    def clear(self):
        """
        Remove all objects from the render window.
        """
        self.remove(*self._results)
        self.append(misc.ChiggerBackground())
        self.update()

    def needsUpdate(self):
        """
        Returns True if the window or any of the child objects require update.
        """
        needs_update = base.ChiggerObject.needsUpdate(self)
        return needs_update or any([result.needsUpdate() for result in self._results])

    def setActive(self, result):
        """
        Set the active result object for interaction.
        """
        if result not in self._results:
            mooseutils.mooseError("The active result must be added to the RendererWindow prior to "
                                  "setting it as active.")
            return

        self.__active = result
        if self.__vtkinteractor:
            self.__vtkinteractor.GetInteractorStyle().SetDefaultRenderer(result.getVTKRenderer())

    def getActive(self):
        """
        Return the active result object.
        """
        return self.__active

    def start(self, timer=None):
        """
        Begin the interactive VTK session.
        """
        if timer:
            msg = "The timer argument is deprecated, please use the 'observers' setting."
            mooseutils.mooseWarning(msg)

        mooseutils.mooseDebug("{}.start()".format(self.__class__.__name__), color='MAGENTA')

        if self.needsUpdate():
            self.update()

        if self.__vtkinteractor:
            self.__vtkinteractor.Initialize()
            self.__vtkinteractor.Start()

        if self.getOption('style') == 'test':
            self.__vtkwindow.Finalize()

    def update(self, **kwargs):
        """
        Updates the child results and renders the results.
        """
        super(RenderWindow, self).update(**kwargs)

        # Setup interactor
        if self.isOptionValid('test') and self.getOption('test'):
            self.__vtkwindow.OffScreenRenderingOn()

        elif self.isOptionValid('style'):
            if self.__vtkinteractor is None:
                self.__vtkinteractor = self.__vtkwindow.MakeRenderWindowInteractor()

            style = self.getOption('style').lower()
            self.setOption('style', None) # avoids calling this function unless it changes
            if style == 'interactive':
                b = base.KeyPressInteractorStyle(self.__vtkinteractor)
                self.__vtkinteractor.SetInteractorStyle(b)
            elif style == 'interactive2d':
                self.__vtkinteractor.SetInteractorStyle(vtk.vtkInteractorStyleImage())
            elif style == 'modal':
                self.__vtkinteractor.SetInteractorStyle(vtk.vtkInteractorStyleUser())

        if self.isOptionValid('motion_factor'):
            self.__vtkinteractor.GetInteractorStyle(). \
                SetMotionFactor(self.getOption('motion_factor'))

        # Background settings
        self._results[0].updateOptions(self._options)

        # vtkRenderWindow Settings
        if self.isOptionValid('offscreen'):
            self.__vtkwindow.SetOffScreenRendering(self.getOption('offscreen'))

        if self.isOptionValid('smoothing'):
            smooth = self.getOption('smoothing')
            self.__vtkwindow.SetLineSmoothing(smooth)
            self.__vtkwindow.SetPolygonSmoothing(smooth)
            self.__vtkwindow.SetPointSmoothing(smooth)

        if self.isOptionValid('size'):
            self.__vtkwindow.SetSize(self.getOption('size'))

        self.__vtkwindow.Render()

        # Setup the result objects
        n = self.__vtkwindow.GetNumberOfLayers()
        for group in self._groups:
            if group.needsUpdate():
                group.update()

        for result in self._results:
            renderer = result.getVTKRenderer()
            if self.isOptionValid('antialiasing'):
                if VTK_MAJOR_VERSION < 8:
                    self.__vtkwindow.SetAAFrames(self.getOption('antialiasing'))
                else:
                    renderer.SetUseFXAA(True)
                    self.__vtkwindow.SetMultiSamples(self.getOption('antialiasing'))

            if not self.__vtkwindow.HasRenderer(renderer):
                self.__vtkwindow.AddRenderer(renderer)
            if result.needsUpdate():
                result.update()
            n = max(n, renderer.GetLayer() + 1)
        self.__vtkwindow.SetNumberOfLayers(n)

        if (self.__active is None) and len(self._results) > 1:
            self.setActive(self._results[1])

        # Observers
        if self.__vtkinteractor:
            for observer in self.getOption('observers'):
                if not isinstance(observer, observers.ChiggerObserver):
                    msg = "The supplied observer of type {} must be a {} object."
                    raise mooseutils.MooseException(msg.format(type(observer),
                                                               observers.ChiggerObserver))

                elif not observer.isActive() is None:
                    observer.init(self)

        self.__vtkwindow.Render()

    def resetCamera(self):
        """
        Resets all the cameras.

        Generally, this is not needed but in some cases when testing the camera needs to be reset
        for the image to look correct.
        """
        for result in self._results:
            result.getVTKRenderer().ResetCamera()

    def resetCameraClippingRange(self):
        """
        Resets the clipping range, this may be needed if you see artifacts in the renderering.
        """
        for result in self._results:
            result.getVTKRenderer().ResetCameraClippingRange()

    def write(self, filename, dialog=False, **kwargs):
        """
        Writes the VTKWindow to an image.
        """
        mooseutils.mooseDebug('RenderWindow.write()', color='MAGENTA')

        if self.needsUpdate() or kwargs:
            self.update(**kwargs)

        # Allowed extensions and the associated readers
        writers = dict()
        writers['.png'] = vtk.vtkPNGWriter
        writers['.ps'] = vtk.vtkPostScriptWriter
        writers['.tiff'] = vtk.vtkTIFFWriter
        writers['.bmp'] = vtk.vtkBMPWriter
        writers['.jpg'] = vtk.vtkJPEGWriter

        # Extract the extensionq
        _, ext = os.path.splitext(filename)
        if ext not in writers:
            w = ', '.join(writers.keys())
            msg = "The filename must end with one of the following extensions: {}.".format(w)
            mooseutils.mooseError(msg, dialog=dialog)
            return

        # Check that the directory exists
        dirname = os.path.dirname(filename)
        if (len(dirname) > 0) and (not os.path.isdir(dirname)):
            msg = "The directory does not exist: {}".format(dirname)
            mooseutils.mooseError(msg, dialog=dialog)
            return

        # Build a filter for writing an image
        window_filter = vtk.vtkWindowToImageFilter()
        window_filter.SetInput(self.__vtkwindow)
        window_filter.Update()

        # Write it
        writer = writers[ext]()
        writer.SetFileName(filename)
        writer.SetInputData(window_filter.GetOutput())
        writer.Write()

    def __getitem__(self, index):
        """
        Operator[] access into results objects.
        """
        return self._results[index]
