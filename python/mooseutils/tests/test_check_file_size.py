#!/usr/bin/env python
import unittest
from mooseutils import check_file_size

class TestCheckFileSize(unittest.TestCase):
    """
    Test that the size function returns something.
    """

    def testBasic(self):
        results = check_file_size(size=1)
        self.assertTrue(results)

if __name__ == '__main__':
    unittest.main(module=__name__, verbosity=2, buffer=True, exit=False)
