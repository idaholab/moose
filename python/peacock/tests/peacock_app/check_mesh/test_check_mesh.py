#!/usr/bin/env python
from peacock.utils import Testing
class Tests(Testing.PeacockTester):
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
        Testing.process_events(self.qapp, t=2)
        Testing.set_window_size(vtkwin)
        Testing.process_events(self.qapp, t=2)
        vtkwin.onWrite(self.filename)
        self.assertFalse(Testing.gold_diff(self.filename))

if __name__ == '__main__':
    Testing.run_tests()
