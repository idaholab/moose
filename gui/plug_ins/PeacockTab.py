try:
    from PyQt4 import QtCore, QtGui
    QtCore.Signal = QtCore.pyqtSignal
    QtCore.Slot = QtCore.pyqtSlot
except ImportError:
    try:
        from PySide import QtCore, QtGui
    except ImportError:
        raise ImportError("Cannot load either PyQt or PySide")

class PeacockTab(QtGui.QWidget):
  def __init__(self, main_window, win_parent=None):
    QtGui.QWidget.__init__(self,win_parent)
    self.main_window = main_window

  def name(self):
    pass

