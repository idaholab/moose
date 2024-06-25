#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html
import functools
import os
from PyQt5 import QtWidgets
from PyQt5.QtGui import QColor, QBrush
from PyQt5.QtCore import pyqtSignal, Qt
from peacock.base.MooseWidget import MooseWidget
from .ParameterInfo import ParameterInfo

class EditingDelegate(QtWidgets.QStyledItemDelegate):
    """
    A simple delegate so we can get notified when the user starts to edit a cell.
    Unfortunately Qt doesn't provide an easy way to do this.
    We want this so that if a user starts editing a cell then we assume
    that it will be changed. So this will enable the "Apply" button once the
    user starts editing a cell.
    We also keep the current editor so that if the user wants to quit while
    in the process of editing a cell, we can grab the current text.
    """
    startedEditing = pyqtSignal()

    def __init__(self, *args, **kwargs):
        super(EditingDelegate, self).__init__(*args, **kwargs)
        self.current_editor = None

    def createEditor(self, parent, option, index):
        """
        Override the default implementation to emit a signal and store the created editor.
        """
        self.startedEditing.emit()
        self.current_editor = super(EditingDelegate, self).createEditor(parent, option, index)
        return self.current_editor

def paramSort(a, b):
    """
    Custom parameter name sorter.
    We want the required parameters first.
    Of the required parameters, we want "Name" to be
    first, then "type". The rest sorted regularly.
    After the required params, the rest of the params
    are sorted normally.
    """
    def cmp(a, b):
        return (a>b)-(a<b)

    if a.required and b.required:
        if a.name == "Name":
            return -1
        elif b.name == "Name" :
            return 1
        elif a.name == "type":
            return -1
        elif b.name == "type" :
            return 1
        else:
            return cmp(a.name, b.name)
    elif a.required:
        return -1
    elif b.required:
        return 1
    else:
        return cmp(a.name, b.name)


class ParamsTable(QtWidgets.QTableWidget, MooseWidget):
    """
    Holds a QTableWidget of parameters.
    There a different kinds of parameters.
    1. Name: The name of the block. This doesn't show up in the normal JSON
        so has to be inserted manually.
    2. User added: This will be colored differently to highlight they are non standard parameters.
    3. Required parameters: If these are not set then this is not valid. These will be
        colored differently than non required parameters.
    Columns:
        Name: Name of the parameter. Can be editable if it is a user parameter.
        Value: The current value of the parameter.
        Options: If the parameter has options those go here. Or for buttons to open file dialogs, etc.
        Comment: A comment on the parameter.

    Signals:
        needBlockList[list]: A list of path names to get children for. This is used where a parameter's
            cpp_type is a reference to another variable. So this gets emitted so that the list of children
            can be updated.
        blockRenamed[BlockInfo, newname]: The block should be renamed
        changed: Whenever an item has changed.
    """
    needBlockList = pyqtSignal(list)
    blockRenamed = pyqtSignal(object, str)
    changed = pyqtSignal()

    def __init__(self, block, params, type_block_map, **kwds):
        """
        Constructor.
        Input:
            block[BlockInfo]: The main block
            params[list[ParameterInfo]]: List of parameters to show
        """
        super(ParamsTable, self).__init__(**kwds)
        self.block = block
        self.params = params
        self.setColumnCount(4)
        self.setHorizontalHeaderLabels(["Name", "Value", "Options", "Comment"])
        self.verticalHeader().hide()
        self.updateSizes()
        self.param_watch_blocks = {}
        self.watch_blocks_params = {}
        self.name_param = None
        self.removed_params = []
        self.type_to_block_map = type_block_map
        for p in sorted(self.params, key=functools.cmp_to_key(paramSort)):
            self.addParam(p)

        self.updateWatchers()
        self.cellChanged.connect(self.onCellChanged)
        self.editing_delegate = EditingDelegate()
        self.setItemDelegate(self.editing_delegate)
        self.editing_delegate.startedEditing.connect(self.changed)

    def save(self):
        """
        Save the values in the text to the parameters.
        """
        # If the user is currently editing a cell we want to grab
        # the current text and set the cell to that value
        if self.state() == QtWidgets.QAbstractItemView.EditingState:
            current_edit = self.editing_delegate.current_editor
            if current_edit and isinstance(current_edit, QtWidgets.QLineEdit):
                current_item = self.currentItem()
                current_item.setText(current_edit.text())

        for i in range(self.rowCount()):
            name_item = self.item(i, 0)
            name = name_item.text()
            p = name_item.data(Qt.UserRole)
            if p.cpp_type == "bool":
                w = self.cellWidget(i, 1)
                value = "false"
                if w.checkState() == Qt.Checked:
                    value = "true"
            else:
                value = self.item(i, 1).text()
            comments = self.item(i, 3).text()
            if p.name == "Name" and p.value != value:
                self.blockRenamed.emit(self.block, value)
            elif p.user_added:
                if p.parent is None:
                    self.block.addParameter(p)
                if p.name != name:
                    self.block.renameUserParam(p.name, name)

            p.value = str(value)
            p.comments = str(comments)
        for p in self.removed_params:
            self.block.removeUserParam(p.name)
        self.removed_params = []
        self.changed.emit()

    def reset(self):
        """
        Reset the text from the values in the parameters.
        """
        rows_removed = []
        for i in range(self.rowCount()):
            name_item = self.item(i, 0)
            p = name_item.data(Qt.UserRole)
            if p.cpp_type == "bool":
                w = self.cellWidget(i, 1)
                if p.value == "true":
                    w.setCheckState(Qt.Checked)
                else:
                    w.setCheckState(Qt.Unchecked)
            else:
                self.item(i, 1).setText(p.value)
            self.item(i, 3).setText(p.comments)
            if p.user_added and p.parent is None:
                rows_removed.append(i)
            elif p.user_added:
                self.item(i, 0).setText(p.name)

        for i in reversed(rows_removed):
            self.removeRow(i)

        for p in self.removed_params:
            self.createRow(p, name_editable=True)

    def updateWatchers(self):
        if self.watch_blocks_params:
            for params in self.watch_blocks_params.values():
                for param in params:
                    row = self.findRow(param)
                    combo = self.cellWidget(row, 2)
                    combo.blockSignals(True)
                    combo.clear()
                    combo.addItem("Select option")
                    combo.setCurrentIndex(0)
                    combo.blockSignals(False)
            self.needBlockList.emit(list(self.watch_blocks_params.keys()))

    def onCellChanged(self, row, col):
        self.changed.emit()

    def setWatchedBlockList(self, path, children):
        """
        For parameters that depend on others, this
        allows the listener to update the acceptable
        values in the options.
        """
        params = self.watch_blocks_params.get(path, [])
        if not params:
            return
        for param in params:
            row = self.findRow(param)
            combo = self.cellWidget(row, 2)
            combo.addItems(children)

    def removeUserParams(self):
        rows_removed = []

        for i in range(self.rowCount()):
            name_item = self.item(i, 0)
            p = name_item.data(Qt.UserRole)
            if p.user_added:
                rows_removed.append(i)

        for i in reversed(rows_removed):
            self.removeRow(i)

    def addUserParams(self, params):
        for p in params:
            self.createRow(p, name_editable=True)

    def getUserParams(self):
        params = []
        for i in range(self.rowCount()):
            name_item = self.item(i, 0)
            p = name_item.data(Qt.UserRole)
            if p.user_added:
                params.append(p)
        return params

    def updateSizes(self):
        """
        Update the sizes of the header.
        """
        header = self.horizontalHeader()
        header.setSectionResizeMode(0, QtWidgets.QHeaderView.ResizeToContents)
        header.setSectionResizeMode(1, QtWidgets.QHeaderView.ResizeToContents)
        header.setSectionResizeMode(2, QtWidgets.QHeaderView.ResizeToContents)
        header.setSectionResizeMode(3, QtWidgets.QHeaderView.Stretch)

    def addParam(self, param, name_editable=False, value_editable=True, index=-1):
        idx = self.findRow(param.name)
        if idx < 0:
            idx = self.createRow(param, name_editable, value_editable, True, index)
            self.updateSizes()
        return idx

    def addName(self, name):
        if self.name_param:
            return
        self.name_param = ParameterInfo(None, "Name")
        self.name_param.value = name
        self.name_param.required = True
        self.createRow(self.name_param, index=0, comments_editable=False)

    def addUserParam(self, name):
        p = ParameterInfo(None, name)
        p.user_added = True
        self.createRow(p, name_editable=True)
        return p

    def findRow(self, param_name):
        for i in range(self.rowCount()):
            if self.item(i, 0).text() == param_name:
                return i
        return -1

    def _paramValue(self, row):
        w = self.cellWidget(row, 1)
        if w:
            if w.checkState() == Qt.Checked:
                return "true"
            else:
                return "false"
        item = self.item(row, 1)
        return item.text()

    def paramValue(self, name):
        row = self.findRow(name)
        if row >= 0:
            return self._paramValue(row)

    def _openFileDialog(self, param, text, use_extension):
        (file_name, filter_name) = QtWidgets.QFileDialog.getOpenFileName(self, text, os.getcwd(), "File (*)")
        if not file_name:
            return
        file_name = os.path.relpath(file_name)
        if not use_extension:
            file_name = os.path.splitext(file_name)[0]

        row = self.findRow(param.name)
        append = param.isVectorType()
        self._updateValue(row, append, file_name)

    def _createFilenameOption(self, param):
        cpp_type_map = {"MeshFileName": "Find Mesh File",
                        "MatrixFileName": "Find Matrix File",
                "FileNameNoExtension": "Find Base File",
                }
        text = cpp_type_map.get(param.cpp_type, "Find File")
        button = QtWidgets.QPushButton(text)
        extension = param.cpp_type != "FileNameNoExtension"
        button.clicked.connect(lambda : self._openFileDialog(param, text, extension))
        row = self.findRow(param.name)
        self.setCellWidget(row, 2, button)

    def updateType(self, type_name):
        idx = self.findRow("type")
        if idx >= 0:
            self._updateValue(idx, False, type_name)

    def _updateValue(self, row, append, val):
        item = self.item(row, 1)
        current_val = val
        if append:
            current_val = item.text()
            if current_val:
                current_val += " " + val
            else:
                current_val = val
        item.setText(current_val)

    def getCurrentParamData(self):
        """
        Get a dctionary of the current state of all parameters.
        This can be different then what the actual ParameterInfo
        objects hold since the user might not have saved yet.
        """
        param_data = {}
        for i in range(self.rowCount()):
            name_item = self.item(i, 0)
            name = name_item.text()
            p = name_item.data(Qt.UserRole)
            comments_item = self.item(i, 3)
            data = {"name": name,
                    "value": self._paramValue(i),
                    "comments": comments_item.text(),
                    "group": p.group_name,
                    "user_added": p.user_added,
                    }
            data["changed"] = data["value"] != p.default or data["comments"]
            param_data[name] = data
        return param_data

    def setParamValue(self, name, val, comments=None):
        """
        Set the value (and optionally comments) for a parameter.
        Input:
            name[str]: Name of the parameter
            val[str]: New value of the parameter
            comments[str]: New comments
        """
        row = self.findRow(name)
        if row >= 0:
            w = self.cellWidget(row, 1)
            if w:
                if val == "true":
                    w.setCheckState(Qt.Checked)
                else:
                    w.setCheckState(Qt.Unchecked)
            else:
                item = self.item(row, 1)
                item.setText(val)
            if comments is not None:
                item = self.item(row, 3)
                item.setText(comments)

    def _optionSelected(self, param_name, append, text):
        row = self.findRow(param_name)
        self._updateValue(row, append, text)
        combo = self.cellWidget(row, 2)
        combo.blockSignals(True)
        combo.setCurrentIndex(0)
        combo.blockSignals(False)

    def _createOptions(self, param, tooltip=""):
        combo = QtWidgets.QComboBox()
        combo.addItem("Select option")
        combo.addItems(param.options)
        combo.setCurrentIndex(0)
        append = param.isVectorType()
        combo.currentTextChanged.connect(lambda text: self._optionSelected(param.name, append, text))
        row = self.findRow(param.name)
        if tooltip:
            combo.setToolTip(tooltip)
        self.setCellWidget(row, 2, combo)

    def _createBlockWatcher(self, param, watch_blocks):
        self.param_watch_blocks[param.name] = watch_blocks
        for b in watch_blocks:
            self.watch_blocks_params.setdefault(b, []).append(param.name)
        tooltip = "Automatically updates from the following %s" % ' '.join(watch_blocks)
        self._createOptions(param, tooltip)

    def _removeButtonClicked(self, name_item):
        row = self.findRow(name_item.text())
        p = name_item.data(Qt.UserRole)
        self.removed_params.append(p)
        self.removeRow(row)
        self.changed.emit()

    def createRow(self, param, name_editable=False, value_editable=True, comments_editable=True, index=-1):
        """
        Create a row in the table for a param.
        Input:
            param: ParamNode
        Return:
            BaseRowItem derived object
        """
        row = self.rowCount()
        if index >= 0:
            row = index
        self.insertRow(row)
        name_item = QtWidgets.QTableWidgetItem(param.name)
        name_item.setData(Qt.UserRole, param)

        if not name_editable:
            name_item.setFlags(Qt.ItemIsEnabled)

        if param.required:
            color = QColor(255, 204, 153)
            brush = QBrush(color)
            name_item.setBackground(brush)
        elif param.user_added:
            color = QColor("cyan")
            brush = QBrush(color)
            name_item.setBackground(brush)

        name_item.setText(param.name)
        self.setItem(row, 0, name_item)

        if param.cpp_type == "bool":
            checkbox = QtWidgets.QCheckBox()
            if param.value == "true":
                checkbox.setCheckState(Qt.Checked)
            else:
                checkbox.setCheckState(Qt.Unchecked)
            checkbox.pressed.connect(self.changed)
            self.setCellWidget(row, 1, checkbox)
        else:
            value_item = QtWidgets.QTableWidgetItem(param.value)
            if not value_editable or param.name == "type":
                value_item.setFlags(Qt.ItemIsEnabled)
            else:
                name_item.setToolTip(param.toolTip())

            self.setItem(row, 1, value_item)

        comments_item = QtWidgets.QTableWidgetItem(param.comments)
        if not comments_editable:
            comments_item.setFlags(Qt.ItemIsEnabled)
        self.setItem(row, 3, comments_item)

        watch_blocks = self._getChildrenOfNodeOptions(param.cpp_type)
        if param.cpp_type == "FileName" or param.cpp_type == "MeshFileName" or param.cpp_type == "MatrixFileName" or param.cpp_type == "FileNameNoExtension":
            self._createFilenameOption(param)
        elif watch_blocks:
            self._createBlockWatcher(param, watch_blocks)
        elif param.options:
            self._createOptions(param)
        elif param.user_added:
            button = QtWidgets.QPushButton("Remove")
            button.clicked.connect(lambda checked: self._removeButtonClicked(name_item))
            self.setCellWidget(row, 2, button)
        else:
            option_item = QtWidgets.QTableWidgetItem()
            option_item.setFlags(Qt.NoItemFlags)
            self.setItem(row, 2, option_item)

    def _getChildrenOfNodeOptions(self, cpp_type):
        """
        See if the cpp_type requires dynamic options based on other nodes.
        Input:
            cpp_type[ParameterInfo.cpp_type]
        Return:
            list of paths that will be used as options, or None
        """
        for key, value in self.type_to_block_map.items():
            # The key (ie something like VectorPostprocessorName) can
            # also be inside a vector
            vector_key = "vector<%s>" % key
            if key == cpp_type or vector_key in cpp_type:
                return value
        return None
