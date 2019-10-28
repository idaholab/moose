#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html
import functools
from . import Testing
from PyQt5.QtTest import QTest
from PyQt5.QtCore import Qt

PROCESS_EVENT_TIME = .2

def changeTableCell(t, param, col, text):
    row = t.findRow(param)
    item = t.item(row, col)
    item.setText(text)

def changeTableCombo(t, param, col, text):
    row = t.findRow(param)
    combo = t.cellWidget(row, col)
    combo.setCurrentText(text)

def clickTableButton(t, param, col):
    row = t.findRow(param)
    button = t.cellWidget(row, col)
    button.click()

class InputTreeTester(object):
    def getTreeWidget(self, w):
        objects = Testing.findQObjectsByType(w, "peacock.Input.InputPathTreeWidget.InputPathTreeWidget")
        self.assertEqual(len(objects), 1)
        return objects[0]

    def clickTab(self, app, tab_name):
        peacock_tab_widgets = Testing.findQObjectsByName(app, "PeacockMainWindow/tab_plugin")
        self.assertEqual(len(peacock_tab_widgets), 1)
        tabs = peacock_tab_widgets[0]
        for i in range(tabs.count()):
            if tabs.tabText(i) == tab_name:
                tab_bar = tabs.tabBar()
                rect = tab_bar.tabRect(i)
                QTest.mouseMove(tab_bar, rect.center())
                QTest.mouseClick(tab_bar, Qt.LeftButton, pos=rect.center(), delay=100)
                Testing.process_events(self.qapp, t=PROCESS_EVENT_TIME)
                return

    def getNodeWidget(self, w, path, old_path=None):
        node = w.InputFileEditorPlugin.tree.getBlockInfo(path)
        self.assertNotEqual(node, None)
        old_path = node.fullPath()
        node_widget = w.InputFileEditorPlugin.getNodeWidgetFromNode(node, old_path)
        return node_widget

    def changeParamTypeCombo(self, w, path, new_type, old_path=None):
        node_widget = self.getNodeWidget(w, path, old_path)
        self.assertNotEqual(node_widget, None)
        type_combo = Testing.findQObjectsByName(node_widget, "type_combo")
        self.assertEqual(len(type_combo), 1)
        target = type_combo[0]
        QTest.mouseMove(target)
        QTest.mouseClick(target, Qt.LeftButton, delay=100)
        # Now the selector should pop up which is a QListView
        list_views = Testing.findQObjectsByType(target, "PyQt5.QtWidgets.QListView")
        self.assertEqual(len(list_views), 1)
        lv = list_views[0]
        model = lv.model()
        matches = model.findItems(new_type)
        self.assertEqual(len(matches), 1)
        model_idx = model.indexFromItem(matches[0])
        rect = lv.visualRect(model_idx)
        QTest.mouseMove(lv, pos=rect.center())
        Testing.process_events(self.qapp, t=PROCESS_EVENT_TIME)
        QTest.mouseClick(lv.viewport(), Qt.LeftButton, pos=rect.center(), delay=100)
        Testing.process_events(self.qapp, t=PROCESS_EVENT_TIME)

    def changeParamCombo(self, w, path, value_name, new_value, old_path=None):
        param_widget = self.getParamWidget(w, path, value_name, old_path)
        self.assertNotEqual(param_widget, None)
        pos = param_widget.valueTestPosition()
        widget = param_widget.valueTestWidget()
        QTest.mouseMove(widget, pos)
        Testing.process_events(self.qapp, t=PROCESS_EVENT_TIME)
        QTest.mouseClick(widget, Qt.LeftButton, pos=pos)
        # Now the selector should pop up which is a QListView
        list_views = Testing.findQObjectsByType(widget, "PyQt5.QtWidgets.QListView")
        self.assertEqual(len(list_views), 1)
        lv = list_views[0]
        model = lv.model()
        matches = model.findItems(new_value)
        self.assertEqual(len(matches), 1)
        model_idx = model.indexFromItem(matches[0])
        rect = lv.visualRect(model_idx)
        QTest.mouseMove(lv, pos=rect.center())
        Testing.process_events(self.qapp, t=PROCESS_EVENT_TIME)
        QTest.mouseClick(lv.viewport(), Qt.LeftButton, pos=rect.center(), delay=100)
        Testing.process_events(self.qapp, t=PROCESS_EVENT_TIME)

    def changeParamValueText(self, w, path, value_name, new_text, old_path=None):
        param_widget = self.getParamWidget(w, path, value_name, old_path)
        self.assertNotEqual(param_widget, None)
        pos = param_widget.valueTestPosition()
        widget = param_widget.valueTestWidget()
        QTest.mouseMove(widget, pos)
        Testing.process_events(self.qapp, t=PROCESS_EVENT_TIME)
        QTest.mouseClick(widget, Qt.LeftButton, pos=pos)
        QTest.mouseDClick(widget, Qt.LeftButton, pos=pos)
        # The default editing widget on a QTreeWidgetItem is a QLineEdit.
        # But it doesn't seem to show up until the item has focus.
        # This is pretty brittle but seems to work OK for now.
        line_edit = Testing.findQObjectsByType(widget, "PyQt5.QtWidgets.QLineEdit")
        for obj in line_edit:
            if obj.objectName() == "":
                widget = obj
                break
        Testing.process_events(self.qapp, t=PROCESS_EVENT_TIME)
        QTest.keyClick(widget, Qt.Key_Backspace)
        QTest.keyClicks(widget, new_text)
        QTest.keyClick(widget, Qt.Key_Return)
        Testing.process_events(self.qapp, t=PROCESS_EVENT_TIME)

    def saveNodeWidget(self, w, path, old_path=None):
        node_widget = self.getNodeWidget(w, path, old_path)
        self.assertNotEqual(node_widget, None)
        apply_button = Testing.findQObjectsByName(node_widget, "apply_button")
        self.assertEqual(len(apply_button), 1)
        self.assertEqual(apply_button[0].isEnabled(), True)
        self.clickButton(apply_button)
        Testing.process_events(self.qapp, t=PROCESS_EVENT_TIME)
        self.assertEqual(apply_button[0].isEnabled(), False)

    def clickButton(self, button_objs):
        self.assertEqual(len(button_objs), 1)
        QTest.mouseMove(button_objs[0])
        QTest.mouseClick(button_objs[0], Qt.LeftButton, delay=100)
        Testing.process_events(self.qapp, t=PROCESS_EVENT_TIME)

    def addToNode(self, w, path, expand=False, include=False):
        if expand:
            self.clickOnTree(w, path, expand=True)
        if include:
            self.clickOnTree(w, path, include=True)
        node_widget = self.getNodeWidget(w, path)
        add_button = Testing.findQObjectsByName(node_widget, "add_button")
        self.assertEqual(add_button[0].isEnabled(), True)
        self.clickButton(add_button)

    def getParamWidget(self, w, path, param_name, old_path=None):
        node_widget = self.getNodeWidget(w, path, old_path)
        self.assertNotEqual(node_widget, None)
        param_widget = node_widget.findParamWidget(param_name)
        self.assertNotEqual(param_widget, None)
        param_widget._table.scrollToItem(param_widget._name_item)
        return param_widget

    def clickOnTree(self, w, path, expand=False, include=False):
        tree = self.getTreeWidget(w)
        item = tree.findPathItem(path)
        self.assertNotEqual(item, None)
        tree.scrollToItem(item)
        tree.setCurrentItem(item)
        rect = tree.visualItemRect(item)
        viewport = tree.viewport()
        pos = rect.center()
        if expand or include:
            pos = rect.bottomLeft()
            pos.setY(pos.y() - rect.height()/2)
        if include:
            pos.setX(pos.x() + 10)

        QTest.mouseMove(tree, pos)
        Testing.process_events(self.qapp, t=PROCESS_EVENT_TIME)
        QTest.mouseClick(viewport, Qt.LeftButton, delay=100, pos=pos)
        Testing.process_events(self.qapp, t=PROCESS_EVENT_TIME)

    def selectMesh(self, w, mesh_type, dim=2, nx=10, ny=10):
        path = "/Mesh"
        self.clickOnTree(w, path, expand=True, include=True)
        self.changeParamTypeCombo(w, path, mesh_type)
        self.changeParamValueText(w, path, "nx", str(nx))
        self.changeParamValueText(w, path, "ny", str(ny))
        self.changeParamCombo(w, path, "dim", "2")
        self.saveNodeWidget(w, path)
        Testing.process_events(self.qapp, t=PROCESS_EVENT_TIME)

    def addVariable(self, w, var_name, include_vars=False, expand_vars=False, include=True):
        self.addToNode(w, "/Variables", expand=expand_vars, include=include_vars)
        new_path = "/Variables/New0"
        if include:
            self.clickOnTree(w, new_path, include=True)
        self.clickOnTree(w, new_path)
        self.changeParamValueText(w, new_path, "name", var_name)
        name_path = "/Variables/%s" % var_name
        self.saveNodeWidget(w, name_path, new_path)
        Testing.process_events(self.qapp, t=PROCESS_EVENT_TIME)

    def addBC(self, w, name, variable, boundary, bc_type="DirichletBC", value="0", include_bc=False, expand_bc=False, include=True):
        self.addToNode(w, "/BCs", expand=expand_bc, include=include_bc)
        new_path = "/BCs/New0"
        if include:
            self.clickOnTree(w, new_path, include=True)
        self.clickOnTree(w, new_path)
        self.changeParamTypeCombo(w, new_path, bc_type)
        self.changeParamValueText(w, new_path, "name", name)
        name_path = "/BCs/%s" % name
        self.changeParamValueText(w, name_path, "value", value, new_path)
        self.changeParamValueText(w, name_path, "boundary", boundary, new_path)
        self.saveNodeWidget(w, name_path, new_path)
        Testing.process_events(self.qapp, t=PROCESS_EVENT_TIME)

    def addKernel(self, w, name, kernel_type, variable, include_kernels=False, expand_kernels=False):
        self.addToNode(w, "/Kernels", expand=expand_kernels, include=include_kernels)
        new_path = "/Kernels/New0"
        if include_kernels:
            self.clickOnTree(w, new_path, include=True)
        self.clickOnTree(w, new_path)
        self.changeParamTypeCombo(w, new_path, kernel_type)
        self.changeParamCombo(w, new_path, "variable", variable)
        self.changeParamValueText(w, new_path, "name", name)
        name_path = "/Kernels/%s" % name
        self.saveNodeWidget(w, name_path, new_path)
        Testing.process_events(self.qapp, t=PROCESS_EVENT_TIME)

    def setExecutioner(self, w, exe_type, solve_type=None, petsc_iname=None, petsc_value=None, include_exe=False, expand_exe=False):
        path = "/Executioner"
        self.clickOnTree(w, path, expand=expand_exe, include=include_exe)
        self.changeParamTypeCombo(w, path, exe_type)
        if petsc_iname:
            self.changeParamValueText(w, path, "petsc_options_iname", petsc_iname)
        if petsc_value:
            self.changeParamValueText(w, path, "petsc_options_value", petsc_value)
        if solve_type:
            self.changeParamCombo(w, path, "solve_type", solve_type)
        self.saveNodeWidget(w, path)
        Testing.process_events(self.qapp, t=PROCESS_EVENT_TIME)

    def changeParamValueToggle(self, w, path, value_name, old_path=None):
        param_widget = self.getParamWidget(w, path, value_name, old_path)
        self.assertNotEqual(param_widget, None)
        pos = param_widget.valueTestPosition()
        widget = param_widget.valueTestWidget()
        QTest.mouseMove(widget, pos)
        Testing.process_events(self.qapp, t=PROCESS_EVENT_TIME)
        QTest.mouseClick(widget, Qt.LeftButton, pos=pos)
        Testing.process_events(self.qapp, t=PROCESS_EVENT_TIME)

    def setOutput(self, w, csv=False, exodus=False):
        path = "/Outputs"
        self.clickOnTree(w, path, expand=True, include=True)
        if exodus:
            self.changeParamValueToggle(w, path, "exodus")
        if csv:
            self.changeParamValueToggle(w, path, "csv")
        Testing.process_events(self.qapp, t=PROCESS_EVENT_TIME)
        self.saveNodeWidget(w, path)
        Testing.process_events(self.qapp, t=PROCESS_EVENT_TIME)

    def createSimpleDiffusion(self, w):
        self.addVariable(w, "u", include_vars=True, expand_vars=True)
        self.selectMesh(w, "GeneratedMesh")
        self.addBC(w, "left", "u", "left", include_bc=True, expand_bc=True)
        self.addBC(w, "right", "u", "right", value="1")
        self.addKernel(w, "diff", "Diffusion", "u", include_kernels=True, expand_kernels=True)
        self.setExecutioner(w, "Steady", solve_type="PJFNK", petsc_iname="-pc_type -pc_hypre_type", petsc_value="hypre boomeramg", include_exe=True, expand_exe=True)
        self.setOutput(w, exodus=True)
