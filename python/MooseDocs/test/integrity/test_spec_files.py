#!/usr/bin/env python2
#pylint: disable=missing-docstring
import os
import sys
import unittest
import glob

import mooseutils

import MooseDocs
from MooseDocs.base import testing

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
            node = mooseutils.hit_load(os.path.join(location, 'tests'))
            for block in node.find('Tests'):
                if block['type'] == 'PythonUnitTest':
                    tested.add(block['input'])

        # Loop through python files in this directory
        for filename in glob.glob(os.path.join(location, '*.py')):

            # Local filename
            base = os.path.basename(filename)

            # Load the module (tried this with os.chdir, but that didn't work)
            sys.path.append(location)
            mod = __import__(base[:-3])
            sys.path.remove(location)

            # Get a lit of unittest.TestCase objects, if they exist this file should be in spec
            tests = testing.get_parent_objects(mod, unittest.TestCase)
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
