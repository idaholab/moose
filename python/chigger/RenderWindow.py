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
import sys
import vtk

import base
import observers
import geometric
import misc
import mooseutils

class ChiggerInteractorStyle(vtk.vtkInteractorStyleUser):
    def OnKeyPress(self, *args, **kwargs):
        print 'hello'
        pass


class RenderWindow(base.ChiggerObject):
    """
    Wrapper of VTK RenderWindow for use with ChiggerResultBase objects.
    """
    RESULT_TYPE = base.ChiggerResultBase

    @staticmethod
    def validOptions():
        opt = base.ChiggerObject.validOptions()

        opt.add('size', default=(960, 540), vtype=int, size=2,
                doc="The size of the window, expects a list of two items")
        opt.add('style', default='interactive', vtype=str,
                allow=('interactive', 'modal', 'interactive2D'),
                doc="The interaction style ('interactive' enables 3D interaction, 'interactive2D' " \
                    "disables out-of-plane interaction, and 'modal' disables all interaction.")
        opt.add('test', default='--test' in sys.argv, vtype=bool,
                doc="When True the interaction is disabled and the window closes immediately " \
                    "after rendering.")
        opt.add('offscreen', default=False, vtype=bool,
                doc="Enable offscreen rendering.")
        #opt.add('chigger', False, "Places a chigger logo in the lower left corner.") #TODO
        opt.add('smoothing', default=False, vtype=bool,
                doc="Enable VTK render window smoothing options.")
        opt.add('multisamples', vtype=int,
                doc="Set the number of multi-samples.")
        opt.add('antialiasing', default=0, vtype=int,
                doc="Number of antialiasing frames to perform (set vtkRenderWindow::SetAAFrames).")
        opt.add('reset_camera', vtype=bool, default=True,
                doc="Automatically reset the camera clipping range on update.")

        # Observers
        opt.add('observers', default=[], vtype=list,
                doc="A list of ChiggerObserver objects, once added they are not " \
                    "removed. Hence, changing the observers in this list will not " \
                    "remove existing objects.")
        opt.add('default_observer',
                default=observers.MainWindowObserver(),
                vtype=observers.ChiggerObserver,
                doc="Define the default observer for the window.")

        # Background settings
        background = misc.ChiggerBackground.validOptions()
        background.remove('layer')
        background.remove('camera')
        background.remove('viewport')
        opt += background
        return opt

    def __init__(self, *args, **kwargs):
        self.__vtkwindow = kwargs.pop('vtkwindow', vtk.vtkRenderWindow())
        self.__vtkinteractor = kwargs.pop('vtkinteractor', None)
        self.__vtkinteractorstyle = None

        super(RenderWindow, self).__init__(**kwargs)

        self._results = []
        self._observers = set()
        self.__active = None
        self.__highlight = None

        # Store the supplied result objects
        self.__background = misc.ChiggerBackground()
        self.append(self.__background, *args)
        self.setActive(None)

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

    def append(self, *args):
        """
        Append result object(s) to the window.
        """
        for result in args:
            mooseutils.mooseDebug('RenderWindow.append {}'.format(type(result).__name__))
            if isinstance(result, base.ResultGroup):
                self.append(*result.getResults())
            elif not isinstance(result, self.RESULT_TYPE):
                n = result.__class__.__name__
                t = self.RESULT_TYPE.__name__
                msg = 'The supplied result type of {} must be of type {}.'.format(n, t)
                raise mooseutils.MooseException(msg)
            self._results.append(result)
            result.init(self)

    def remove(self, *args):
        """
        Remove result object(s) from the window.
        """
        for result in args:
            result.deinit()
            if result.getVTKRenderer().GetActors().GetNumberOfItems() == 0:
                self.__vtkwindow.RemoveRenderer(result.getVTKRenderer())
            if result in self._results:
                self._results.remove(result)

        # Reset active if it was removed
        #if self.__active and (self.__active not in self._results):
        #    self.setActive(None)

        self.update()

    def clear(self):
        """
        Remove all objects from the render window.
        """
        self.remove(*self._results[1:])
        self.update()

    def setActive(self, result):
        """
        Set the active result object for interaction.
        """

        # Deactivate current active result, if it exists and differs
        if (self.__active is not None) and (self.__active is not result):
            self.__active.setActive(False)

        # Deactivate all results, this is done by using the background renderer
        if result is None:
            self.__active = None
            self.__background.getVTKRenderer().SetInteractive(True)
            if self.getVTKInteractorStyle():
                self.getVTKInteractorStyle().SetCurrentRenderer(self.__background.getVTKRenderer())
                self.getVTKInteractorStyle().SetEnabled(False)

        # Activate the supplied result
        else:
            self.__active = result
            self.__background.getVTKRenderer().SetInteractive(False)
            self.getVTKInteractorStyle().SetEnabled(True)
            self.getVTKInteractorStyle().SetCurrentRenderer(self.__active.getVTKRenderer())
            self.__active.setActive(True)
            #if self.getOption('highlight_active'):
            #    print 'Activate: {}'.format(self.__active.title())

    def nextActive(self, reverse=False):
        """
        Highlight the next ChiggerResult object.
        """
        step = 1 if not reverse else -1
        available = [result for result in self._results if result.getOption('interactive')]
        if (self.__active is None) and step == 1:
            self.setActive(available[0])
        elif (self.__active is None) and step == -1:
            self.setActive(available[-1])
        else:
            n = len(available)
            index = available.index(self.__active)
            index += step
            if (index == n) or (index == -1):
                self.setActive(None)
            else:
                self.setActive(available[index])

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

        self.update()

        if self.__vtkinteractor:
            self.__vtkinteractor.Initialize()
            self.__vtkinteractor.Start()

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
                self.__vtkinteractorstyle = vtk.vtkInteractorStyleJoystickCamera()
            elif style == 'interactive2d':
                self.__vtkinteractorstyle = vtk.vtkInteractorStyleImage()
            elif style == 'modal':
                self.__vtkinteractorstyle = vtk.vtkInteractorStyleUser()

            self.__vtkinteractor.SetInteractorStyle(self.__vtkinteractorstyle)
            self.__vtkinteractor.RemoveObservers(vtk.vtkCommand.CharEvent)

            main_observer = self.getOption('default_observer')
            if main_observer is not None:
                main_observer.init(self)
                self._observers.add(main_observer)

        # Background settings
        self._results[0].update(background=self._options.get('background'),
                                background2=self._options.get('background2'),
                                gradient_background=self._options.get('gradient_background'))

        # vtkRenderWindow Settings
        if self.isOptionValid('offscreen'):
            self.__vtkwindow.SetOffScreenRendering(self.getOption('offscreen'))

        if self.isOptionValid('smoothing'):
            smooth = self.getOption('smoothing')
            self.__vtkwindow.SetLineSmoothing(smooth)
            self.__vtkwindow.SetPolygonSmoothing(smooth)
            self.__vtkwindow.SetPointSmoothing(smooth)

        if self.isOptionValid('antialiasing'):
            self.__vtkwindow.SetAAFrames(self.getOption('antialiasing'))

        if self.isOptionValid('multisamples'):
            self.__vtkwindow.SetMultiSamples(self.getOption('multisamples'))

        if self.isOptionValid('size'):
            self.__vtkwindow.SetSize(self.applyOption('size'))

        # Setup the result objects
        n = self.__vtkwindow.GetNumberOfLayers()
        for result in self._results:
            renderer = result.getVTKRenderer()
            if not self.__vtkwindow.HasRenderer(renderer):
                self.__vtkwindow.AddRenderer(renderer)
            result.update()
            n = max(n, renderer.GetLayer() + 1)

        # TODO: set if changed only
        self.__vtkwindow.SetNumberOfLayers(n)

        # Observers
        if self.__vtkinteractor:

            for observer in self.getOption('observers'):
                if not isinstance(observer, observers.ChiggerObserver):
                    msg = "The supplied observer of type {} must be a {} object."
                    raise mooseutils.MooseException(msg.format(type(observer),
                                                               observers.ChiggerObserver))

                if observer not in self._observers:
                    observer.init(self)
                    self._observers.add(observer)

        for result in self._results:
            result.update()

        self.__vtkwindow.Render()

        if self.getOption('reset_camera'):
            for result in self._results:
                if result.isOptionValid('camera'):
                    result.getVTKRenderer().ResetCameraClippingRange()
                else:
                    result.getVTKRenderer().ResetCamera()

    def resetCamera(self):
        """
        Resets all the cameras.

        Generally, this is not needed but in some cases when testing the camera needs to be reset
        for the image to look correct.
        """
        for result in self._results:
            result.getVTKRenderer().ResetCamera()

    def resetClippingRange(self):
        """
        Resets all the clipping range for open cameras.
        """
        for result in self._results:
            result.getVTKRenderer().ResetCameraClippingRange()

    def write(self, filename, dialog=False, **kwargs):
        """
        Writes the VTKWindow to an image.
        """
        mooseutils.mooseDebug('RenderWindow.write()', color='MAGENTA')
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
        if self.__background.getOption('background') is None:
            window_filter.SetInputBufferTypeToRGBA()
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
