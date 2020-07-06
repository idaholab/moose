#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import os
import sys
import unittest
import glob
import mooseutils
import pyhit
import moosetree
import inspect
import MooseDocs

def get_parent_objects(module, cls):
    """Tool for locating all objects that derive from a certain base class."""
    func = lambda obj: inspect.isclass(obj) and issubclass(obj, cls)
    return inspect.getmembers(module, predicate=func)

class TestSpecFiles(unittest.TestCase):
    def check(self, location):

        # List of errors
        messages = []

        # Load the test spec and create a list of PythonUnitTest files
        tested = set()
        spec = os.path.join(location, 'tests')
        if not os.path.exists(spec):
            if glob.glob(os.path.join(spec, '*.py')):
                messages.append("Missing a test spec file in '{}'".format(os.path.dirname(spec)))
        else:
            node = pyhit.load(os.path.join(location, 'tests'))

            # check for PythonUnitTest blocks in [Tests]
            block = moosetree.find(node, lambda n: n.name=='Tests')
            for subblock in moosetree.findall(block, lambda n: n):
                if subblock['type'] == 'PythonUnitTest':
                    tested.add(subblock['input'])

        # Loop through python files in this directory
        for filename in glob.glob(os.path.join(location, '*.py')):

            # Local filename
            base = os.path.basename(filename)

            # Load the module (tried this with os.chdir, but that didn't work)
            sys.path.append(location)
            mod = __import__(base[:-3])
            sys.path.remove(location)

            # Get a lit of unittest.TestCase objects, if they exist this file should be in spec
            tests = get_parent_objects(mod, unittest.TestCase)
            if tests and (base not in tested):
                msg = "The test script '{}' is not included in the tests spec '{}'."
                messages.append(msg.format(base, spec))

        return messages

    def testSpec(self):
        messages = []
        location = os.path.join(os.path.dirname(MooseDocs.__file__), 'test')
        for root, dirs, _ in os.walk(location):
            for d in dirs:
                messages += self.check(os.path.join(root, d))

        self.assertFalse(messages, '\n' + '\n'.join(messages))

if __name__ == '__main__':
    unittest.main(verbosity=2)
