from Plugin import Plugin
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
