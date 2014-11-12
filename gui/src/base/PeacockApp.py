import sys

from PySide import QtCore, QtGui

from src.base import PeacockTabWidget

class PeacockApp(object):
  def __init__(self, **kwargs):

    self.app = QtGui.QApplication(sys.argv)
    self.main = QtGui.QMainWindow()
    self.menubar = QtGui.QMenuBar() # need parentless menu bar for OSX (see PySide QMainWindow.menuBar())
    self.main.setMenuBar(self.menubar)

    screen_rect = self.app.desktop().screenGeometry()
    width, height = screen_rect.width(), screen_rect.height()

    self.tabs = PeacockTabWidget(main=self.main, alignment='vertical',
                                 screen_width=width, screen_height=height, **kwargs)

    self.tabs.show()

    sys.exit(self.app.exec_())
