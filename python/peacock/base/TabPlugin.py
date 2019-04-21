#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from .Plugin import Plugin
class TabPlugin(Plugin):
    def __init__(self):
        super(TabPlugin, self).__init__()
        self._name = self.__class__.__name__

    def setTabName(self, name):
        self._name = name

    def tabName(self):
        return self._name

    def initialize(self, options):
        """
        Initialize the TabPlugin with command-line arguments.

        Inputs:
            options[Namespace]: Command-line arguments returned from argparse
        """
        pass
