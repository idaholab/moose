from PyQt4 import QtCore, QtGui

class PeacockTab(QtGui.QWidget):
  def __init__(self, main_window, win_parent=None):
    QtGui.QWidget.__init__(self,win_parent)
    self.main_window = main_window

  def name(self):
    pass
  
