from Plugin import Plugin
class TabPlugin(Plugin):
    def __init__(self):
        super(TabPlugin, self).__init__()
        self._name = self.__class__.__name__

    def setTabName(self, name):
        self._name = name

    def tabName(self):
        return self._name
