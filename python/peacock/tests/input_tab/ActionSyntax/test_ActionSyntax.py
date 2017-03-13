#!/usr/bin/env python
from peacock.Input.ActionSyntax import ActionSyntax
from peacock.utils import Testing

class Tests(Testing.PeacockTester):
    def testBadExe(self):
        s = ActionSyntax("/no_exist")
        self.assertEqual(s.app_path, None)
        s.appChanged("/no_exist2")
        self.assertEqual(s.app_path, None)

    def testGoodExe(self):
        path = Testing.find_moose_test_exe()
        s = ActionSyntax(path)
        self.assertEqual(s.app_path, path)
        self.assertEqual(s.isHardPath("/Adaptivity"), True)
        self.assertEqual(s.isHardPath("/AuxVariables"), True)
        self.assertEqual(s.isHardPath("/AuxVariables/foo"), False)
        self.assertEqual(s.isHardPath("/AuxVariables/foo/InitialCondition"), True)
        self.assertEqual(s.isHardPath("/Variables/*/InitialCondition"), True)
        self.assertEqual(s.isHardPath("/Variables/foo/InitialCondition"), True)
        self.assertEqual(s.isHardPath("/Mesh/Partitioner"), True)

        p = s.getPath("/Variables/foo")
        self.assertEqual(p, None)

        p = s.getPath("/Variables/foo/InitialCondition")
        self.assertEqual(p, "Variables/*/InitialCondition")

    def testPickle(self):
        path = Testing.find_moose_test_exe()
        s = ActionSyntax(path)
        p = s.toPickle()
        s2 = ActionSyntax()
        s2.fromPickle(p)
        self.assertEqual(s2.app_path, s.app_path)
        self.assertEqual(s2.paths, s.paths)
        self.assertEqual(s2.hard_paths, s.hard_paths)
        self.assertEqual(s2.hard_path_patterns, s.hard_path_patterns)

    def testStar(self):
        path = Testing.find_moose_test_exe()
        s = ActionSyntax(path)
        self.assertEqual(s.isStar("/Adaptivity"), False)
        self.assertEqual(s.isStar("/AuxKernels"), True)
        self.assertEqual(s.isStar("/Functions"), True)
        self.assertEqual(s.isStar("/Preconditioning"), True)
        self.assertEqual(s.isStar("/Preconditioning/foo"), True)

if __name__ == '__main__':
    Testing.run_tests()
