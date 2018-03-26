#!/usr/bin/env python2
#pylint: disable=missing-docstring

import os
import sys
import unittest
import importlib

import MooseDocs
from MooseDocs.extensions import core
from MooseDocs.base import testing, components
from MooseDocs.tree import tokens

class TestExtensions(testing.MooseDocsTestCase):
    """
    Tests that Extension objects have the

    """
    EXTENSIONS = [core]

    REQUIRED_TOKENIZE = set(['Test{}Tokenize'])
    REQUIRED_RENDERER = set(['Test{}HTML', 'Test{}Materialize', 'Test{}Latex'])

    @classmethod
    def setUpClass(cls):
        ext = os.path.join(MooseDocs.MOOSE_DIR, 'python', 'MooseDocs', 'test', 'extensions')
        sys.path.append(ext)

    def testTokenComponents(self):
        self.checkTestCases(components.TokenComponent, self.REQUIRED_TOKENIZE)

    def testRenderComponents(self):
        self.checkTestCases(components.RenderComponent, self.REQUIRED_RENDERER)

    def testTokens(self):
        messages = []
        for mod in TestExtensions.EXTENSIONS:
            name = mod.__name__.split('.')[-1]
            objects = testing.get_parent_objects(mod, tokens.Token)

            try:
                tmod = importlib.import_module('test_{}'.format(name))
            except ImportError:
                msg = "test_{0}.py must be added to test/extensions, run './moosedocs.py devel " \
                      "--generate-extension-tests {0}'".format(name)
                messages.append(msg)
                continue

            case = None
            for case_name, case_type in testing.get_parent_objects(tmod, unittest.TestCase):
                if case_name == 'TestTokens':
                    case = case_type

            if case is None:
                msg = 'TestTokens must be created in test_{}.py'.format(name)
                messages.append(msg)
                continue

            testcases = dir(case)
            for obj in objects:
                test_case = 'test{}'.format(obj[0])
                if test_case not in testcases:
                    msg = "{} method must be added to TestTokens in test_{}.py".format(test_case, name)
                    messages.append(msg)

        self.assertFalse(messages, '\n' + '\n'.join(messages))


    def checkTestCases(self, obj_type, required):
        messages = []
        for mod in TestExtensions.EXTENSIONS:
            mod_name = mod.__name__.split('.')[-1]
            objects = testing.get_parent_objects(mod, obj_type)

            try:
                tmod = importlib.import_module('test_{}'.format(mod_name))
            except ImportError:
                msg = "test_{0}.py must be added to test/extensions, run './moosedocs.py devel " \
                      "--generate-extension-tests {0}'".format(mod_name)
                messages.append(msg)
                continue

            testcases = [name for name, _ in testing.get_parent_objects(tmod, unittest.TestCase)]
            for req in required:
                for obj in objects:
                    test_case = req.format(obj[0])
                    if test_case not in testcases:
                        msg = "{} unittest.TestCase must be added to test_{}.py"
                        messages.append(msg.format(obj[0], mod_name))

        self.assertFalse(messages, '\n' + '\n'.join(messages))

if __name__ == '__main__':
    unittest.main(verbosity=2)
