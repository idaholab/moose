#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import sys
import os
from PyQt5 import QtCore, QtWidgets
import vtk
from vtk.qt.QVTKRenderWindowInteractor import QVTKRenderWindowInteractor
import chigger
import mooseutils
from .ExodusPlugin import ExodusPlugin

class RetinaQVTKRenderWindowInteractor(QVTKRenderWindowInteractor):
    """
    Currently VTK7.1 and Qt5 do not work correctly on retina displays:
    http://public.kitware.com/pipermail/vtk-developers/2017-February/034738.html

    However, creating a custom resizeEvent method and wrapping the QVTKRenderWindowInteractor object
    in a QFrame allowed it to work for now. The idea for this wrapping came from:
        https://github.com/siudej/Eigenvalues/blob/master/qvtk.py
    """
    def resizeEvent(self, event):
        """
        Double the size on retina displays.
        This is not the right way to do it, but this works for framed widgets.
        We also need to modify all mouse events to adjust the interactor's
        center (e.g. for joystick mode).
        """
        super(RetinaQVTKRenderWindowInteractor, self).resizeEvent(event)

        ratio = self.devicePixelRatio()
        w = self.width()
        h = self.height()
        if (self.parent() is not None) and (w <= self.parent().width()):
            self.resize(ratio*self.size())
        self.GetRenderWindow().SetSize(ratio*w, ratio*h)
        self.GetRenderWindow().GetInteractor().SetSize(ratio*w, ratio*h)
        self.GetRenderWindow().GetInteractor().ConfigureEvent()
        self.update()

class VTKWindowPlugin(QtWidgets.QFrame, ExodusPlugin):
    """
    Plugin for volume rendering of ExodusII data with VTK via chigger.
    """

    #: pyqtSignal: Emitted when the result is first rendered
    setupResult = QtCore.pyqtSignal(chigger.exodus.ExodusResult)

    #: pyqtSignal: Emitted when the reader has been update, prior to creating result
    setupReader = QtCore.pyqtSignal(chigger.exodus.ExodusReader)

    #: pyqtSignal: Emitted when the window has been created
    setupWindow = QtCore.pyqtSignal(chigger.RenderWindow)

    #: pyqtSignal: Emitted with the window has been updated
    updateWindow = QtCore.pyqtSignal(chigger.RenderWindow,
                                     chigger.exodus.ExodusReader,
                                     chigger.exodus.ExodusResult)

    #: pyqtSignal: Emitted when the window is reset/cleared
    resetWindow = QtCore.pyqtSignal()

    #: pyqtSignal: Emitted when the Exodus widgets should be enabled/disabled
    setEnableWidget = QtCore.pyqtSignal(bool)

    #: pyqtSignal: Emitted when the camera for this window has changed
    cameraChanged = QtCore.pyqtSignal(tuple, tuple, tuple)

    @staticmethod
    def getDefaultReaderOptions():
        """
        Return the default options for the ExodusReader object.
        """
        return dict()

    @staticmethod
    def getDefaultResultOptions():
        """
        Return the default options for the ExodusResult object.
        """
        return dict()

    @staticmethod
    def getDefaultWindowOptions():
        """
        Return the default options for the RenderWindow object.
        """
        return dict()

    def __init__(self, size=None, **kwargs):
        super(VTKWindowPlugin, self).__init__(**kwargs)

        # Setup widget
        self.setSizePolicy(QtWidgets.QSizePolicy.MinimumExpanding,
                           QtWidgets.QSizePolicy.MinimumExpanding)

        # Create the QVTK interactor
        if self.devicePixelRatio() > 1:
            self.__qvtkinteractor = RetinaQVTKRenderWindowInteractor(self)
        else:
            self.__qvtkinteractor = QVTKRenderWindowInteractor(self)

        # Member variables
        self._highlight = None
        self._initialized = False
        self._run_start_time = -1
        self._reader = None
        self._result = None
        self._window = chigger.RenderWindow(vtkwindow=self.__qvtkinteractor.GetRenderWindow(),
                                            vtkinteractor=self.__qvtkinteractor.GetRenderWindow().GetInteractor(),
                                            size=size)
        self._reader_options = self.getDefaultReaderOptions()
        self._result_options = self.getDefaultResultOptions()
        self._window_options = self.getDefaultWindowOptions()

        # If size is provided, restrict the window
        if size != None:
            self.setFixedSize(QtCore.QSize(*size))
        else:
            self.setMinimumSize(QtCore.QSize(600, 600))

        # Define timers for initialization and updating data
        self._timers = dict()
        for name in ['update', 'initialize']:
            self._timers[name] = QtCore.QTimer()
            self._timers[name].setInterval(1000)
        self._timers['update'].timeout.connect(self.onWindowRequiresUpdate)
        self._timers['initialize'].timeout.connect(self.onWindowRequiresUpdate)

        # Create a "tight" layout and add the QVTK widget
        self._layout = QtWidgets.QHBoxLayout()
        self._layout.setContentsMargins(0, 0, 0, 0)
        self._layout.addWidget(self.__qvtkinteractor)
        self.setLayout(self._layout)

        # Camera cache
        self._cameras = dict()

        # Display items for when no file exists
        self._peacock_logo = chigger.annotations.ImageAnnotation(filename='peacock.png', opacity=0.33)
        self._peacock_text = chigger.annotations.TextAnnotation(justification='center', font_size=18)

        self.setMainLayoutName('RightLayout')
        self.setup()

    def onSetFilename(self, filename):
        """
        Slot for changing the filename, this is generally called from the FilePlugin signal.
        """
        # Variable and component are stored in the result options, thus are not needed
        if filename != self._filename:
            self._filename = filename
            self._run_start_time = -1
            self._reset()

    def onSetVariable(self, variable):
        """
        Slot for changing the variable, this is generally called from the FilePlugin signal.
        """
        super(VTKWindowPlugin, self).onSetVariable(variable)
        self.onReaderOptionsChanged({'variable':self._variable})
        self.onResultOptionsChanged({'variable':self._variable})

    def onSetComponent(self, component):
        """
        Slot for changing the variable component, this is generally called from the FilePlugin signal.
        """
        super(VTKWindowPlugin, self).onSetComponent(component)
        self.onResultOptionsChanged({'component':self._component})

    def onReaderOptionsChanged(self, options):
        """
        Update the options for ExodusReader object.
        """
        self.__setOptionsHelper(self._reader, options, self._reader_options)

    def onResultOptionsChanged(self, options=dict()):
        """
        Update the options for ExodusResult object.
        """
        self.__setOptionsHelper(self._result, options, self._result_options)

    def onWindowOptionsChanged(self, options=dict()):
        """
        Update the options for RenderWindow object.
        """
        self.__setOptionsHelper(self._window, options, self._window_options)

    def onWindowRequiresUpdate(self):
        """
        Updates the VTK render window, if needed.

        This is what should be called after changes to this plugin have been made.

        This is the only slot that actually causes a render to
        happen. The other slots should be used to setup the window,
        then this called to actually preform the update. This avoids
        performing multiple updates to the window.
        """

        file_exists = os.path.exists(self._filename) if self._filename else False
        if file_exists and (os.path.getmtime(self._filename) < self._run_start_time):
            self._reset()
            msg = '{} is currently out of date.\nIt will load automatically when it is updated.'
            self._setLoadingMessage(msg.format(self._filename))

        if (not self._initialized) and file_exists and (os.path.getsize(self._filename) > 0) and \
           (os.path.getmtime(self._filename) >= self._run_start_time):
            self._renderResult()

        elif (self._filename is not None) and (not file_exists):
            self._reset()
            msg = '{} does not currently exist.\nIt will load automatically when it is created.'
            self._setLoadingMessage(msg.format(self._filename))

        elif (self._filename is None):
            self._reset()
            self._setLoadingMessage('No file selected.')

        if self._window.needsUpdate():# and (self._reader is not None):
            self._window.update()

            for result in self._window:
                result.getVTKRenderer().DrawOn()

            if self._reader is not None:
                self.updateWindow.emit(self._window, self._reader, self._result)

            if self._reader is not None:
                err = self._reader.getErrorObserver()
                if err:
                    mooseutils.mooseDebug('Failed to update VTK window.', traceback=True)

    def onCameraChanged(self, view, position, focal):
        """
        Accepts camera settings.

        Inputs:
            view[tuple]: vtkCamera::ViewUp
            position[tuple]: vtkCamera::SetPosition
            focal[tuple]: vtkCamera::FocalPoint
        """
        if self._result:
            camera = self._result.getVTKRenderer().GetActiveCamera()
            camera.SetViewUp(view)
            camera.SetPosition(position)
            camera.SetFocalPoint(focal)
            self._result.getVTKRenderer().ResetCameraClippingRange()

    def onInputFileChanged(self, filename):
        """
        Force window to reset on the next update b/c the input file has changed.
        """
        self._reset()
        self.onWindowRequiresUpdate()

    def onJobStart(self, csv, path, t):
        """
        Update the 'run' time to avoid loading old data.
        """
        self._run_start_time = t
        self.onWindowRequiresUpdate()

    def onAddFilter(self, filter_):
        """
        Adds supplied filter to the result object.
        """
        if self._result:
            filters = self._result.getOption('filters')
            if filter_ not in filters:
                filters.append(filter_)
                self.onResultOptionsChanged({'filters':filters})

    def onRemoveFilter(self, filter_):
        """
        Removes the supplied filter from the result object.
        """
        if self._result:
            filters = self._result.getOption('filters')
            if filter_ in filters:
                filters.remove(filter_)
                self.onResultOptionsChanged({'filters':filters})

    def onCurrentChanged(self, index):
        """
        Called when the tab is changed.

        Inputs:
            index[int]: The index of the active tab.
        """
        active = self._index == index

        # If the tab is active and initialized then start auto updating
        if active and self._initialized:
            self._adjustTimers(start=['update'], stop=['initialize'])

        # If the tab is active and not initialized then start the initialize timer
        elif active and not self._initialized:
            self._adjustTimers(start=['initialize'], stop=['update'])

        # Turn off times if the tab is not active
        else:
            self._adjustTimers(stop=['initialize', 'update'])

    def onWrite(self, filename):
        """
        Produce a *.png image of the figure.
        """
        if filename.endswith('.py'):
            self.parent().write(filename)
        else:
            self._window.write(filename, dialog=True)

    def onAddObserver(self, event, callback):
        """
        Add a VTK observer callback.
        """
        self._window.getVTKInteractor().AddObserver(event, callback)

    def _reset(self):
        """
        Clears the VTK windows and restarts the initialize timer.
        """
        if self._initialized:

            # Clear all data
            self._window.reset()
            self._window.clear()
            self._adjustTimers(start=['initialize'], stop=['update'])
            self._result = None
            self._reader = None
            self._initialized = False
            self._highlight = None
            self._reader_options = self.getDefaultReaderOptions()
            self._result_options = self.getDefaultResultOptions()
            self._window_options = self.getDefaultWindowOptions()
            self.setEnabled(False)
            self.resetWindow.emit()
            self.setEnableWidget.emit(False)

    def _renderResult(self):
        """
        Render the VTK window with the current file and reader/result/window options (protected).

        This should not be called directly, use the onWindowRequiresUpdate slot
        """
        # Clear any-existing VTK objects on the window
        self._window.clear()
        self._initialized = True

        # Read ExodusII file
        self._reader = chigger.exodus.ExodusReader(self._filename, **self._reader_options)
        self.setupReader.emit(self._reader)
        self._reader.update()

        # Create result (renderer)
        self._result = chigger.exodus.ExodusResult(self._reader, **self._result_options)
        self.setupResult.emit(self._result)
        self._window.append(self._result)
        self._result.update()

        # Set the interaction mode (2D/3D)
        bmin, bmax = self._result.getBounds()
        if abs(bmax[-1] - bmin[-1]) < 1e-10:
            self._window.setOption('style', 'interactive2D')
        else:
            self._window.setOption('style', 'interactive')

        # Connect the camera to the render event
        self.onAddObserver(vtk.vtkCommand.RenderEvent, self._callbackRenderEvent)

        # Update the RenderWindow
        self._window.setOptions(**self._window_options)
        self.setupWindow.emit(self._window)
        self._window.update()

        # Store/load camera
        data = self._cameras.get(self._filename, None)
        if data is not None:
            self.onCameraChanged(*data)

        self._adjustTimers(start=['update'], stop=['initialize'])
        self.setEnabled(True)
        self.setEnableWidget.emit(True)

    def _setLoadingMessage(self, msg):
        """
        Set the text shown when there isn't a file.
        """
        self._peacock_text.update(text=msg)
        if self._peacock_logo not in self._window:
            self._window.append(self._peacock_logo)
            self._window.append(self._peacock_text)

    def _adjustTimers(self, start=[], stop=[]):
        """
        Helper method for starting/stopping timers.
        """
        for s in start:
            self._timers[s].start()
        for s in stop:
            self._timers[s].stop()

    def repr(self):
        """
        Produce a script for reproducing the VTK figure.
        """
        # The content to return
        output = dict()

        window_options, window_sub_options = self._window.options().toScriptString()
        output['window'] = ['window = chigger.RenderWindow(result)']
        output['window'] += ['window.setOptions({})'.format(', '.join(window_options))]
        for key, value in window_sub_options.iteritems():
            output['window'] += ['window.setOptions({}, {})'.format(repr(key), ', '.join(value))]
        output['window'] += ['window.start()']

        # Get the reader and result options
        reader_options, reader_sub_options = self._reader.options().toScriptString()
        result_options, result_sub_options = self._result.options().toScriptString()

        # Remove filters (this is handled by the ExodusPluginManager)
        for opt in result_options:
            if opt.startswith('filters='):
                result_options.remove(opt)

        # Define the imports
        output['imports'] = ['import vtk', 'import chigger']

        # Define the camera
        output['camera'] = ['camera = vtk.vtkCamera()']
        output['camera'] += chigger.utils.print_camera(self._result.getVTKRenderer().GetActiveCamera())
        result_options.append('camera=camera')

        output['reader'] = ['reader = chigger.exodus.ExodusReader({})'.format(repr(os.path.relpath(self._reader.filename())))]
        if reader_options:
            output['reader'] += ['reader.setOptions({})'.format(', '.join(reader_options))]
        for key, value in reader_sub_options.iteritems():
            output['reader'] += ['reader.setOptions({}, {})'.format(repr(key), ', '.join(value))]

        output['result'] = ['result = chigger.exodus.ExodusResult(reader)']
        if result_options:
            output['result'] += ['result.setOptions({})'.format(', '.join(result_options))]
        for key, value in result_sub_options.iteritems():
            output['result'] += ['result.setOptions({}, {})'.format(repr(key), ', '.join(value))]

        return output

    def _callbackRenderEvent(self, *args):
        """
        Emits cameraChanged signal when the camera is modified.
        """
        if self._result:
            camera = self._result.getVTKRenderer().GetActiveCamera()

            # Ideally this would use the GetModelTransformMatrix(), but in python the set method
            # for this function does not appear to work.
            data = (camera.GetViewUp(), camera.GetPosition(), camera.GetFocalPoint())
            self._cameras[self._filename] = data
            self.cameraChanged.emit(*data)

    def __setOptionsHelper(self, chigger_object, options, fallback):
        """
        Private helper for setting chigger options.

        Inputs:
            chigger_object[ChiggerObject]: An object supporting setOptions calls.
            options[dict | chigger.utils.Options]: The options to set.
        """
        if chigger_object:
            if isinstance(options, dict):
                chigger_object.setOptions(**options)
            elif isinstance(options, chigger.utils.Options):
                chigger_object.setOptions(options)
            else:
                raise mooseutils.MooseException('Options supplied must be a dict or utils.Options class.')

        else:
            fallback.update(options)

def main(size=None):
    """
    Run the VTKWindowPlugin all by its lonesome.
    """
    from peacock.ExodusViewer.ExodusPluginManager import ExodusPluginManager
    widget = ExodusPluginManager(plugins=[lambda: VTKWindowPlugin(size=size)])
    widget.show()
    return widget, widget.VTKWindowPlugin

if __name__ == "__main__":
    from peacock.utils import Testing
    app = QtWidgets.QApplication(sys.argv)
    filename = Testing.get_chigger_input('mug_blocks_out.e')
    filename = 'none.e'
    widget, window = main(size=[600,600])
    #window.onSetFilename(filename)
    #window.onSetVariable('diffused')
    #window.onWindowRequiresUpdate()
    sys.exit(app.exec_())
