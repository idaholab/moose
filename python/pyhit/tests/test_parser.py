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
import unittest
import pyhit

class TestHitLoad(unittest.TestCase):
    """
    Test the load function.
    """

    def testRender(self):
        root = pyhit.load(os.path.join('..', '..', 'test_files', 'test.hit'))
        out = root.render()
        self.assertIn('[A]', out)
        self.assertIn('param = bar', out)
        self.assertIn('comment', out)

    def testBasic(self):
        root = pyhit.load(os.path.join('..', '..', 'test_files', 'test.hit'))

        self.assertEqual(root.children[0].name, 'A')
        self.assertEqual(root.children[0]['param'], 'foo')
        self.assertEqual(root.children[0].children[0].name, 'A-1')
        self.assertIn('param', root.children[0].children[0])
        self.assertEqual(root.children[0].children[0]['param'], 'bar')

        self.assertEqual(root.children[1].name, 'B')
        self.assertEqual(root.children[1].children[0].name, 'B-1')
        self.assertEqual(root.children[1].children[0].children[0].name, 'B-1-1')
        self.assertIn('type', root.children[1].children[0].children[0])
        self.assertEqual(root.children[1].children[0].children[0]['type'], 'test')
        self.assertEqual(root.children[1].children[1].name, 'B-2')

        gold = ['A', 'B']
        for i, child in enumerate(root):
            self.assertEqual(child.name, gold[i])

    def testIterParam(self):
        root = pyhit.load(os.path.join('..', '..', 'test_files', 'test.hit'))
        for k, v in root.children[0].params():
            self.assertEqual(k, 'param')
            self.assertEqual(v, 'foo')

    def testGetParam(self):
        root = pyhit.load(os.path.join('..', '..', 'test_files', 'test.hit'))
        p = root.get('nope', 'default')
        self.assertEqual(p, 'default')

    def testAddParam(self):
        root = pyhit.load(os.path.join('..', '..', 'test_files', 'test.hit'))
        self.assertEqual(len(root(1)), 2)
        self.assertIsNone(root(1).get('year'))
        root(1)['year'] = 1980
        self.assertEqual(len(root(1)), 2)
        self.assertEqual(root(1).get('year'), 1980)

    def testEditParam(self):
        root = pyhit.load(os.path.join('..', '..', 'test_files', 'test.hit'))
        self.assertEqual(len(root(1)), 2)
        self.assertIsNone(root(1).get('year'))
        root(1)['year'] = 1980
        self.assertEqual(len(root(1)), 2)
        self.assertEqual(root(1).get('year'), 1980)
        root(1)['year'] = 1949
        self.assertEqual(len(root(1)), 2)
        self.assertEqual(root(1).get('year'), 1949)

    def testRemoveParam(self):
        root = pyhit.load(os.path.join('..', '..', 'test_files', 'test.hit'))
        self.assertEqual(root(0)['param'], 'foo')
        root(0).removeParam('param')
        self.assertIsNone(root(0).get('param'))

    def testAppend(self):
        root = pyhit.load(os.path.join('..', '..', 'test_files', 'test.hit'))
        self.assertEqual(len(root(1)), 2)
        sec = root(1).append('B-3')
        self.assertEqual(len(root(1)), 3)
        self.assertIs(root(1)(2), sec)
        self.assertIsNone(sec.get('year'))
        sec['year'] = 1980
        self.assertEqual(sec.get('year'), 1980)

    def testInsert(self):
        root = pyhit.load(os.path.join('..', '..', 'test_files', 'test.hit'))
        self.assertEqual(len(root(1)), 2)
        sec = root(1).insert(0, 'B-3')
        self.assertEqual(len(root(1)), 3)
        self.assertIs(root(1)(0), sec)
        self.assertIsNone(sec.get('year'))
        sec['year'] = 1980
        self.assertEqual(sec.get('year'), 1980)

    def testAppendWithKwargs(self):
        root = pyhit.load(os.path.join('..', '..', 'test_files', 'test.hit'))
        self.assertEqual(len(root(1)), 2)
        sec = root(1).append('B-3', year=1980)
        self.assertEqual(len(root(1)), 3)
        self.assertEqual(sec.get('year'), 1980)

    def testRemoveSection(self):
        root = pyhit.load(os.path.join('..', '..', 'test_files', 'test.hit'))
        self.assertEqual(len(root), 2)
        root(1).remove()
        self.assertEqual(len(root), 1)

    def testAddSectionWithParameters(self):
        root = pyhit.load(os.path.join('..', '..', 'test_files', 'test.hit'))
        self.assertEqual(len(root(1)), 2)
        sec = root(1).append('B-3', year=1980)
        self.assertEqual(len(root(1)), 3)
        self.assertEqual(sec.get('year'), 1980)

    def testComment(self):
        root = pyhit.load(os.path.join('..', '..', 'test_files', 'test.hit'))
        self.assertEqual(root(1).comment(), "section comment")
        self.assertEqual(root(0,0).comment(), "sub-section comment")
        self.assertEqual(root(1,0,0).comment('type'), "param comment")

    def testSetComment(self):
        root = pyhit.load(os.path.join('..', '..', 'test_files', 'test.hit'))
        root(1).setComment('update section comment')
        self.assertEqual(root(1).comment(), "update section comment")

        root(0,0).setComment('update sub-section comment')
        self.assertEqual(root(0,0).comment(), "update sub-section comment")

        root(1,0,0).setComment('type', 'update param comment')
        self.assertEqual(root(1,0,0).comment('type'), "update param comment")

    def testAddComment(self):
        root = pyhit.load(os.path.join('..', '..', 'test_files', 'test.hit'))

        root(0).setComment('Section A')
        self.assertEqual(root(0).comment(), "Section A")

        root(1,0).setComment('Section B-1')
        self.assertEqual(root(1,0).comment(), "Section B-1")

        root(0,0).setComment('param', "inline comment")
        self.assertEqual(root(0,0).comment('param'), "inline comment")

    def testRemoveComment(self):
        root = pyhit.load(os.path.join('..', '..', 'test_files', 'test.hit'))
        self.assertIn("type = test # param comment", root.render())
        self.assertIn("# section comment", root.render())

        root(1,0,0).setComment("type", None)
        self.assertIsNone(root(1,0,0).comment())
        self.assertNotIn("type = test # param comment", root.render())

        root(1).setComment(None)
        self.assertIsNone(root(1).comment())
        self.assertNotIn("# section comment", root.render())

    def testCreate(self):
        root = pyhit.Node()
        bcs = root.append('BCs')
        bcs.append('left', type='NeumannBC', value=1980, boundary='left')
        self.assertEqual(len(root), 1)
        out =  root.render()
        self.assertIn('[BCs]', out)
        self.assertIn('type = NeumannBC', out)
        self.assertIn('boundary = left', out)

    def testTestRoot(self):
        root = pyhit.load(os.path.join('..', '..', 'testroot'))
        self.assertIn('app_name', root)
        self.assertEqual(root['app_name'], 'moose_python')


if __name__ == '__main__':
    unittest.main(module=__name__, verbosity=2)
