#!/usr/bin/env python
from peacock.utils.ExeLauncher import runExe
from peacock.PeacockException import FileExistsException, BadExecutableException
from peacock.utils import Testing

class Tests(Testing.PeacockTester):
    def test_runexe(self):
        with self.assertRaises(FileExistsException):
            runExe("/no_exist.exe", "")

        out = runExe("/bin/echo", "output")
        self.assertEqual(out, "output\n")

        out = runExe("/bin/echo", ["output"])
        self.assertEqual(out, "output\n")

        with self.assertRaises(BadExecutableException):
            runExe("false", "")


if __name__ == '__main__':
    Testing.run_tests()
