import sys
import os
from PyQt5 import QtCore, QtWidgets
import vtk
from vtk.qt.QVTKRenderWindowInteractor import QVTKRenderWindowInteractor
import chigger
import mooseutils
from ExodusPlugin import ExodusPlugin

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
    windowCreated = QtCore.pyqtSignal(chigger.exodus.ExodusReader, chigger.exodus.ExodusResult, chigger.RenderWindow)

    #: pyqtSignal: Emitted with the window has been updated
    windowUpdated = QtCore.pyqtSignal()

    #: pyqtSignal: Emitted when the window is reset/cleared
    windowReset = QtCore.pyqtSignal()

    #: pyqtSignal: Emitted when the camera for this window has changed
    cameraChanged = QtCore.pyqtSignal(vtk.vtkCamera)

    def __init__(self, size=None):
        super(VTKWindowPlugin, self).__init__()

        # Setup widget
        self.setSizePolicy(QtWidgets.QSizePolicy.Expanding, QtWidgets.QSizePolicy.Expanding)
        self.setMainLayoutName('WindowLayout')

        # Create the QVTK interactor
        if self.devicePixelRatio() > 1:
            self.__qvtkinteractor = RetinaQVTKRenderWindowInteractor(self)
        else:
            self.__qvtkinteractor = QVTKRenderWindowInteractor(self)
        self.__qvtkinteractor.hide()

        # Member variables
        self._highlight = None
        self._reader = None
        self._result = None
        self._filename = None
        self._initialized = False
        self._run_start_time = None
        self._window = chigger.RenderWindow(vtkwindow=self.__qvtkinteractor.GetRenderWindow(), vtkinteractor=self.__qvtkinteractor.GetRenderWindow().GetInteractor(), size=size)

        # Set to True when the window needs to be reset (e.g., when the input file was changed)
        self._reset_required = False

        # If size is provided, restrict the window
        if size != None:
            self.setFixedSize(QtCore.QSize(*size))

        # Define timers for initialization and updating data
        self._timers = dict()
        for name in ['update', 'initialize']:
            self._timers[name] = QtCore.QTimer()
            self._timers[name].setInterval(1000)
        self._timers['update'].timeout.connect(self.onWindowRequiresUpdate)
        self._timers['initialize'].timeout.connect(self.onReloadWindow)

        # Create a "tight" layout and add the QVTK widget
        self._layout = QtWidgets.QHBoxLayout()
        self._layout.setContentsMargins(0, 0, 0, 0)
        self._layout.addWidget(self.__qvtkinteractor)
        self.setLayout(self._layout)

        self.setup()

    def reset(self):
        """
        Clears the VTK windows and restarts the initialize timer.
        """
        self._window.clear()
        self._window.update()
        self._initialized = False
        self._reset_required = False
        self._adjustTimers(start=['initialize'], stop=['update'])
        self.__qvtkinteractor.hide()
        self.windowReset.emit()

    def onReloadWindow(self):
        """
        Reloads the current file.
        """
        if self._filename:
            self.onFileChanged(self._filename)

    def initialize(self, *args, **kwargs):
        """
        Assumes that first file in the supplied list should be loaded, if a FilePlugin is not present.

        Input:
            filenames[list]: (optional) List of filenames to initialize the VTK window with, the first will be displayed
                             and only if a FilePlugin is not present.
        """
        if len(args) == 1 and isinstance(args[0], list) and not hasattr(self.parent(), 'FilePlugin'):
            self.onFileChanged(args[0][0])

    def onFileChanged(self, filename):
        """
        Initialize the VTK window to read and display a file.

        If the file is not valid a timer is started to continue to attempt to initialize the window. When a result
        is found and rendered, the windowCreated signal is emitted containing the chigger reader and result.

        Inputs:
            filename[str]: The filename to open.
        """
        if (filename != self._filename) or self._reset_required:
            self.reset()

        # Do nothing if the widget is not visible or the file doesn't exist
        self._filename = filename
        file_exists = os.path.exists(self._filename)
        if not self.isVisible() or not file_exists:
            return

        # Determine if the file and GUI are in a valid state for rendering result
        if file_exists and self._run_start_time:
            if os.path.getmtime(self._filename) < self._run_start_time:
                self.reset()
                return

        self.__qvtkinteractor.show()
        # Call the base class initialization (this enables the plugin)
        super(VTKWindowPlugin, self).initialize()

        # Clear any-existing VTK objects on the window
        self._window.clear()

        # Create the reader and result chigger objects
        self._reader = chigger.exodus.ExodusReader(filename)
        self._result = chigger.exodus.ExodusResult(self._reader)

        # Set the interaction mode (2D/3D)
        self._result.update()
        bmin, bmax = self._result.getBounds(check=sys.platform=='darwin')
        if abs(bmax[-1] - bmin[-1]) < 1e-10:
            self._window.setOption('style', 'interactive2D')
        else:
            self._window.setOption('style', 'interactive')

        # Add results
        self._window.append(self._result)

        # Connect the camera to the modified event
        self._result.getVTKRenderer().GetActiveCamera().AddObserver(vtk.vtkCommand.ModifiedEvent, self._cameraModifiedCallback)

        # Update the RenderWindow
        self._initialized = True
        self._window.resetCamera() # this needs to be here to get the objects to show up correctly, I have no idea why.
        self._window.update()
        self._adjustTimers(start=['update'], stop=['initialize'])
        self.windowCreated.emit(self._reader, self._result, self._window)

    def onInputFileChanged(self, *args):
        """
        Force window to reset on the next update b/c the input file has changed.
        """
        self._reset_required = True

    def onJobStart(self, csv, path, t):
        """
        Update the 'run' time to avoid loading old data.
        """
        self._run_start_time = t
        self.onReloadWindow()

    def showEvent(self, *args):
        """
        Override the widgets showEvent to load or update the window when it becomes visible
        """
        if not self._initialized:
            self.onReloadWindow()
        else:
            self.onWindowRequiresUpdate()

    def onReaderOptionsChanged(self, options=dict()):
        """
        Update the options for ExodusReader object.
        """
        self.__setOptionsHelper(self._reader, options)

    def onResultOptionsChanged(self, options=dict()):
        """
        Update the options for ExodusResult object.
        """
        self.__setOptionsHelper(self._result, options)

    def onWindowOptionsChanged(self, options=dict()):
        """v
        Update the options for RenderWindow object.
        """
        self.__setOptionsHelper(self._window, options)

    def onAppendResult(self, result):
        """
        Appends a result object (e.g., ColorBar) to the ExodusWindow object
        """
        self._window.append(result)

    def onRemoveResult(self, result):
        """
        Removes a result object (e.g., ColorBar) to the ExodusWindow object
        """
        self._window.pop(result)

    def resizeEvent(self, event):
        """
        Reset the camera for the colorbar so it positioned correctly.

        This is QWidget method that is called when the window is resized.

        Args:
            event[QResizeEvent]: Not used
        """
        super(VTKWindowPlugin, self).resizeEvent(event)
        if self._result and not self._window.needsInitialize():
            try:
                self._result.update()
            except OSError:
                pass # no file exists0

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

    def onWindowRequiresUpdate(self, *args):
        """
        Updates the VTK render window.
        """

        if not self._initialized:
            return

        # Try to preform an update, if the file disappears startup the initialization timer again and remove results
        try:
            if self._window.needsUpdate():
                self._window.update()
                self.windowUpdated.emit()
        except Exception:
            mooseutils.mooseDebug('Failed to update VTK window.', traceback=True)
            self.reset()

    def onCameraChanged(self, camera):
        """
        Update the camera, this will be connected to the cameraChanged signal from other VTKWindowPlugin instances.
        """
        if self._result:
            self._result.getVTKRenderer().GetActiveCamera().DeepCopy(camera)
            self._window.update()

    def onHighlight(self, block=None, boundary=None, nodeset=None):
        """
        Highlight the desired block/boundary/nodeset.

        To remove highlight call this function without the inputs set.

        Args:
            block[list]: List of block ids to highlight.
            boundary[list]: List of boundary ids to highlight.
            nodeset[list]: List of nodeset ids to highlight.
        """
        if not self._highlight:
            self._highlight = chigger.exodus.ExodusResult(self._reader, renderer=self._result.getVTKRenderer(), color=[1,0,0])

        if block or boundary or nodeset:
            self._highlight.setOptions(block=block, boundary=boundary, nodeset=nodeset)
            self._highlight.setOptions(edges=True, edge_width=3, edge_color=[1,0,0])
            self.onAppendResult(self._highlight)
        else:
            self._highlight.reset()
            self.onRemoveResult(self._highlight)

        self.onWindowRequiresUpdate()

    def onWrite(self, filename):
        """
        Produce a *.png image of the figure.
        """
        if filename.endswith('.py'):
            self.parent().write(filename)
        else:
            self._window.write(filename, dialog=True)

    def _adjustTimers(self, start=[], stop=[]):
        """
        Helper method for starting/stoping timers.
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

    def _cameraModifiedCallback(self, *args):
        """
        Emits cameraChanged signal when the camera is modified.
        """
        self.cameraChanged.emit(self._result.getVTKRenderer().GetActiveCamera())

    def __setOptionsHelper(self, chigger_object, options):
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
    widget, window = main(size=[600,600])
    window.initialize([filename])
    window._result.update(variable='diffused')
    window.onWindowRequiresUpdate()
    sys.exit(app.exec_())
