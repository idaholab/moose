#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

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
        self._timer = None
        timer = kwargs.pop('timer', None)
        if timer != None:
            self._timer = QtCore.QTimer()
            self._timer.timeout.connect(self.load)
            self._timer.setInterval(timer)

    def __call__(self, keys, **kwargs):
        if hasattr(self._reader, 'times'):
            self._reader.update(**kwargs)
        else:
            self._reader.update()
        return self._reader[keys]

    def __bool__(self):
        return bool(self._reader)

    def setTimerActive(self, active):
        if active and self._timer:
            self._timer.start()
        elif self._timer:
            self._timer.stop()

    def times(self):
        attr = getattr(self._reader, 'times', None)
        if attr:
            return attr()
        return []

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
