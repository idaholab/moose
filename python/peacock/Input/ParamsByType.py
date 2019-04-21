#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from PyQt5.QtWidgets import QWidget, QComboBox, QStackedWidget
from PyQt5.QtCore import pyqtSignal
from peacock.base.MooseWidget import MooseWidget
from peacock.utils import WidgetUtils
from .ParamsByGroup import ParamsByGroup

class ParamsByType(QWidget, MooseWidget):
    """
    Has a QComboBox for the different allowed types.
    On switching type a new ParamsByGroup is shown.
    """
    needBlockList = pyqtSignal(list)
    blockRenamed = pyqtSignal(object, str)
    changed = pyqtSignal()

    def __init__(self, block, type_block_map, **kwds):
        """
        Constructor.
        Input:
            block[BlockInfo]: The block to show.
        """
        super(ParamsByType, self).__init__(**kwds)
        self.block = block
        self.combo = QComboBox()
        self.types = []
        self.type_params_map = {}
        self.type_block_map = type_block_map
        self.table_stack = QStackedWidget()
        self.type_table_map = {}

        for t in sorted(self.block.types.keys()):
            self.types.append(t)
            params_list = []
            for p in self.block.parameters_list:
                params_list.append(self.block.parameters[p])
            t_block = self.block.types[t]
            for p in t_block.parameters_list:
                params_list.append(t_block.parameters[p])
            self.type_params_map[t] = params_list

        self.combo.addItems(sorted(self.block.types.keys()))
        self.combo.currentTextChanged.connect(self.setBlockType)

        self.top_layout = WidgetUtils.addLayout(vertical=True)
        self.top_layout.addWidget(self.combo)
        self.top_layout.addWidget(self.table_stack)
        self.setLayout(self.top_layout)
        self.user_params = []
        self.setDefaultBlockType()

        self.setup()

    def _syncUserParams(self, current, to):
        """
        Sync user added parameters that are on the main block into
        each type ParamsByGroup.
        Input:
            current[ParamsByGroup]: The current group parameter table
            to[ParamsByGroup]: The new group parameter table
        """
        ct = current.findTable("Main")
        tot = to.findTable("Main")
        if not ct or not tot or ct == tot:
            return
        tot.removeUserParams()
        params = ct.getUserParams()
        tot.addUserParams(params)
        to.syncParamsFrom(current)
        # Make sure the name parameter stays the same
        idx = ct.findRow("Name")
        if idx >= 0:
            name = ct.item(idx, 1).text()
            idx = tot.findRow("Name")
            if idx >= 0:
                tot.item(idx, 1).setText(name)

    def currentType(self):
        return self.combo.currentText()

    def save(self):
        """
        Look at the user params in self.block.parameters.
        update the type tables
        Save type on block
        """
        t = self.getTable()
        if t:
            t.save()
            self.block.setBlockType(self.combo.currentText())

    def reset(self):
        t = self.getTable()
        t.reset()

    def getOrCreateTypeTable(self, type_name):
        """
        Gets the table for the type name or create it if it doesn't exist.
        Input:
            type_name[str]: Name of the type
        Return:
            ParamsByGroup: The parameters corresponding to the type
        """
        t = self.type_table_map.get(type_name)
        if t:
            return t
        t = ParamsByGroup(self.block, self.type_params_map.get(type_name, self.block.orderedParameters()), self.type_block_map)
        t.needBlockList.connect(self.needBlockList)
        t.blockRenamed.connect(self.blockRenamed)
        t.changed.connect(self.changed)
        self.type_table_map[type_name] = t
        self.table_stack.addWidget(t)
        return t

    def setDefaultBlockType(self):
        param = self.block.getParamInfo("type")
        if param and param.value:
            self.setBlockType(param.value)
        elif self.block.types:
            self.setBlockType(sorted(self.block.types.keys())[0])

    def setBlockType(self, type_name):
        if type_name not in self.block.types:
            return
        t = self.getOrCreateTypeTable(type_name)
        t.updateWatchers()
        self.combo.blockSignals(True)
        self.combo.setCurrentText(type_name)
        self.combo.blockSignals(False)
        t.updateType(type_name)
        current = self.table_stack.currentWidget()
        self._syncUserParams(current, t)
        self.table_stack.setCurrentWidget(t)
        self.changed.emit()

    def addUserParam(self, param):
        t = self.table_stack.currentWidget()
        t.addUserParam(param)

    def setWatchedBlockList(self, path, children):
        for i in range(self.table_stack.count()):
            t = self.table_stack.widget(i)
            t.setWatchedBlockList(path, children)

    def updateWatchers(self):
        for i in range(self.table_stack.count()):
            t = self.table_stack.widget(i)
            t.updateWatchers()

    def getTable(self):
        return self.table_stack.currentWidget()

    def paramValue(self, name):
        for i in range(self.table_stack.count()):
            t = self.table_stack.widget(i)
            if t.paramValue(name):
                return t.paramValue(name)
