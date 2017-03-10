from PyQt5.QtWidgets import QTabWidget
from PyQt5.QtCore import pyqtSignal
from peacock.base.MooseWidget import MooseWidget
from ParamsTable import ParamsTable

def tabSort(val):
    """
    Ugly way to make sure Main group is first followed
    by the rest in alphabetical order
    """
    if val == "Main":
        return "0000_Main"
    else:
        return "1111_%s" % val

class ParamsByGroup(QTabWidget, MooseWidget):
    """
    Creates tabs for each group in the parameters
    The default group is "Main"
    Signals:
        needBlockList[list]: List of paths that we need children of
        blockRenamed[BlockInfo, newname]: The block has been renamed
        changed: A value has changed
    """
    needBlockList = pyqtSignal(list)
    blockRenamed = pyqtSignal(object, str)
    changed = pyqtSignal()

    def __init__(self, block, params, **kwds):
        """
        Constructor.
        Input:
            block[BlockInfo]: The block that the parameters belong to.
            params[list[ParameterInfo]]: A list of parameters to show
        """
        super(ParamsByGroup, self).__init__(**kwds)
        self.params = params
        self.block = block

        group_param_map = {}
        self.group_table_map = {}

        for p in self.params:
            group = p.group_name
            params = group_param_map.get(group, [])
            params.append(p)
            group_param_map[group] = params

        for g in sorted(group_param_map.keys(), key=tabSort):
            t = ParamsTable(self.block, group_param_map[g])
            t.needBlockList.connect(self.needBlockList)
            t.blockRenamed.connect(self.blockRenamed)
            t.changed.connect(self.changed)
            self.addTab(t, g)
            self.group_table_map[g] = t
            t.updateWatchers()

        main = self.group_table_map.get("Main")
        self.setCurrentWidget(main)
        if self.block.user_added:
            main.addName(self.block.name)
        self.setup()

    def addName(self, name):
        main = self.group_table_map.get("Main")
        main.addName(name)

    def updateType(self, type_name):
        main = self.group_table_map.get("Main")
        main.updateType(type_name)

    def addUserParam(self, param):
        main = self.group_table_map.get("Main")
        return main.addUserParam(param)

    def setWatchedBlockList(self, path, children):
        for i in range(self.count()):
            t = self.widget(i)
            t.setWatchedBlockList(path, children)

    def updateWatchers(self):
        for i in range(self.count()):
            t = self.widget(i)
            t.updateWatchers()

    def save(self):
        for i in range(self.count()):
            t = self.widget(i)
            t.save()
        self.changed.emit()

    def reset(self):
        for i in range(self.count()):
            t = self.widget(i)
            t.reset()

    def findTable(self, group):
        return self.group_table_map.get(group)

    def paramValue(self, name):
        for i in range(self.count()):
            t = self.widget(i)
            if t.paramValue(name):
                return t.paramValue(name)
