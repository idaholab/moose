#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from peacock.utils import Testing
from PyQt5 import QtWidgets

class Tests(Testing.PeacockTester):
    qapp = QtWidgets.QApplication([])

    def setUp(self):
        super(Tests, self).setUp()
        self.filename = "check_mesh.png"
        Testing.remove_file(self.filename)

    def testInputFileMesh(self):
        exe_file = Testing.find_moose_test_exe()
        app = self.createPeacockApp(['../../common/transient.i', exe_file])
        tabs = app.main_widget.tab_plugin
        input_plugin = tabs.InputFileEditorWithMesh
        self.assertEqual(str(tabs.tabText(tabs.currentIndex())), input_plugin.tabName())
        vtkwin = input_plugin.vtkwin
        Testing.process_events(t=2)
        Testing.set_window_size(vtkwin)
        Testing.process_events(t=2)
        vtkwin.onWrite(self.filename)
        self.assertFalse(Testing.gold_diff(self.filename))

if __name__ == '__main__':
    Testing.run_tests()
