#!/usr/bin/env python
from peacock.Input.YamlData import YamlData
from peacock.utils import Testing

class Tests(Testing.PeacockTester):
    def setUp(self):
        super(Tests, self).setUp()
        self.yaml_changed_count = 0
        self.test_path = "/Variables/*/InitialCondition"

    def check(self, y):
        entry = y.findPath(self.test_path)
        if y.app_path:
            self.assertNotEqual(entry, None)
            out = y.dumpNames()
            self.assertIn(self.test_path, out)
        else:
            self.assertEqual(entry, None)

    def testYamlData(self):
        y = YamlData()
        self.check(y)
        y.appChanged("No exist")
        self.check(y)
        path = Testing.find_moose_test_exe()
        y.appChanged(path)
        self.check(y)

    def testPickle(self):
        y = YamlData()
        self.check(y)
        path = Testing.find_moose_test_exe()
        y.appChanged(path)
        self.check(y)
        p = y.toPickle()
        y2 = YamlData()
        self.check(y2)
        y2.fromPickle(p)
        self.check(y2)
        self.assertEqual(y2.app_path, y.app_path)
        self.assertEqual(y2.yaml_data, y.yaml_data)
        self.assertEqual(y2.all_paths, y.all_paths)

if __name__ == '__main__':
    Testing.run_tests()
