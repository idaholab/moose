from PyQt5 import QtCore

class PostprocessorDataWidget(QtCore.QObject):
    """
    A wrapper for the VectorPostprocessor/PostprocessorReader classes to allow for automatic reloading.

    Args:
        reader[PostprocessorReader or VectorPostprocessorReader]: The data reader object.
        args[tuple]: Arguments passed to QObjects constructor.

    Kwargs:
        timer[int]: When set the timer will execute the load method with the given milliseconds.

    """

    #: pyqtSignal: Emits when data has been reloaded
    dataChanged = QtCore.pyqtSignal()

    def __init__(self, reader, *args, **kwargs):
        QtCore.QObject.__init__(self, *args)

        # Store the reader
        self._reader = reader

        # Reload Timer
        timer = kwargs.pop('timer', None)
        if timer != None:
            self._timer = QtCore.QTimer()
            self._timer.timeout.connect(self.load)
            self._timer.setInterval(timer)

    def __call__(self, keys, **kwargs):
        return self._reader(keys, **kwargs)

    def __nonzero__(self):
        return bool(self._reader)

    def setTimerActive(self, active):
        if active:
            self._timer.start()
        else:
            self._timer.stop()

    def times(self):
        return self._reader.times()

    def filename(self):
        return self._reader.filename

    def variables(self):
        return self._reader.variables()

    def load(self):
        """
        Calls the PostprocessorReader.load and emits a signal.
        """
        self.blockSignals(True)
        rcode = self._reader.update()
        self.blockSignals(False)
        if rcode != 0:
            self.dataChanged.emit()
        return rcode

    def repr(self):
        return self._reader.repr()
