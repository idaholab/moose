from PySide import QtCore, QtGui

from src.base import *
from src.execute import *
#from src.input import *

class PeacockTabWidget(QtGui.QWidget, MooseWidget):
  def __init__(self, **kwargs):
    QtGui.QWidget.__init__(self)
    MooseWidget.__init__(self, **kwargs)

    split = self.addObject(QtGui.QSplitter(), handle='MainVerticalSplit')

    # Create Tab layout
    tabs = self.addObject(QtGui.QTabWidget(), handle='PeacockTabs', parent='MainVerticalSplit')

    # Add the tabs to the layout
#    self.addObject(InputWidget(**kwargs), handle='Input', parent='PeacockTabs')
    self.addObject(ExecuteWidget(**kwargs), handle='Execute', parent='PeacockTabs')

    # Add the Interactive console
    self.addObject(PeacockConsoleWidget(globals(), **kwargs), handle='InteractiveConsole', parent='MainVerticalSplit')

    # Connect the signal 'button' from the InputWidget to the 'Run' callback of ExecuteWidget
#    self.connectSignal('button', 'Run')
#    self.object('Execute').info()

    self.setup()

  def _setupMainVerticalSplit(self, q_object):
    q_object.setOrientation(QtCore.Qt.Vertical)

    h = self.property('screen_height')
    if self.property('debug'):
      q_object.setSizes([0.25*h, 0.25*h])
    else:
      q_object.setSizes([0.25*h, 0])
