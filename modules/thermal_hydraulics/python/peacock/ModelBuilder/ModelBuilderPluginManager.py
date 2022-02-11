#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from PyQt5 import QtWidgets
import peacock

class ModelBuilderPluginManager(QtWidgets.QWidget, peacock.base.PluginManager):
    """
    Plugin manager for component parameter plugins
    """

    def __init__(self, plugins=[]):
        super(ModelBuilderPluginManager, self).__init__(plugins=plugins, plugin_base=peacock.base.Plugin)

        self.setSizePolicy(QtWidgets.QSizePolicy.Preferred, QtWidgets.QSizePolicy.Preferred)
        self.MainLayout = QtWidgets.QHBoxLayout(self)
        self.LeftLayout = QtWidgets.QVBoxLayout()
        self.RightLayout = QtWidgets.QVBoxLayout()
        self.MainLayout.addLayout(self.LeftLayout)
        self.MainLayout.addLayout(self.RightLayout)
        self.setup()
        self.LeftLayout.addStretch(1)

        # Set the width of the left-side widgets to that the VTK window gets the space
        self.fixLayoutWidth('LeftLayout')
