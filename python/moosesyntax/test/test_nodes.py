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
import mock
from moosesyntax.nodes import NodeBase, ObjectNodeBase, SyntaxNode, ActionNode, MooseObjectNode, MooseObjectActionNode

class TestNodeBase(unittest.TestCase):
    def setUp(self):
        self._root = NodeBase(None, '')
        a = NodeBase(self._root, 'a', group='A')
        b = NodeBase(self._root, 'b')
        c = NodeBase(self._root, 'c', group='C')
        NodeBase(a, 'aa', hidden=True)
        NodeBase(a, 'ab', removed=True)
        NodeBase(b, 'ba', parameters={'year':1980}, description='description')
        NodeBase(b, 'bb', alias='cc')
        NodeBase(c, 'ca', attrib='foo')
        NodeBase(c, 'cb', group='B')

    def testProperties(self):
        r = self._root

        self.assertEqual(r(0,0).hidden, True)
        self.assertEqual(r(0,1).hidden, False)

        self.assertEqual(r(0,0).removed, False)
        self.assertEqual(r(0,1).removed, True)

        self.assertEqual(r(1,0).alias, None)
        self.assertEqual(r(1,1).alias, 'cc')

        self.assertEqual(r(2,0).group, None)
        self.assertEqual(r(2,1).group, 'B')

        self.assertEqual(r.color, 'RED')

        self.assertEqual(r.markdown, None)

    def testFullpath(self):
        r = self._root
        self.assertEqual(r(1).fullpath(), '/b')
        self.assertEqual(r(1,0).fullpath(), '/b/ba')
        self.assertEqual(r(1,1).fullpath(), '/b/bb')

    def testGroups(self):
        r = self._root
        self.assertEqual(r.groups(), set())
        self.assertEqual(r(0).groups(), {'A'})
        self.assertEqual(r(2).groups(), {'C'})
        self.assertEqual(r(2,1).groups(), {'B'})

    @mock.patch('mooseutils.colorText', side_effect=lambda t, c: '{}:{}'.format(c,t))
    def testRepr(self, *args):
        r = self._root
        self.assertIn('LIGHT_RED:b: /b LIGHT_GREY:hidden=False removed=False group=None groups=set() test=False alias=None', repr(r(1)))
        self.assertIn('GREY:ab: /a/ab GREY:hidden=False removed=True group=None groups=set() test=False alias=None', repr(r(0,1)))
        self.assertIn('RED:aa: /a/aa LIGHT_GREY:hidden=True removed=False group=None groups=set() test=False alias=None', repr(r(0,0)))
        self.assertIn('attrib', r(2,0))

class TestSyntaxNode(unittest.TestCase):
    def setUp(self):
        self._root = SyntaxNode(None, 'a')
        self._syntax = [SyntaxNode(self._root, 's0'), SyntaxNode(self._root, 's1')]
        self._objects = [MooseObjectNode(self._root, 'mo0'),
                         MooseObjectNode(self._root, 'mo1')]
        self._actions = [ActionNode(self._root, 'a0', parameters={'year':1980}),
                         MooseObjectActionNode(self._root, 'a1', parameters={'month':6})]

    def testProperties(self):
        r = self._root
        self.assertEqual(r(0).markdown, 'a/s0/index.md')

    def testMarkdown(self):
        r = ObjectNodeBase(None, 'a', markdown='andrew.md')
        self.assertEqual(r.markdown, 'andrew.md')

    def testParameters(self):
        params = self._root.parameters()
        self.assertIn('year', params)
        self.assertEqual(params['year'], 1980)
        self.assertIn('month', params)
        self.assertEqual(params['month'], 6)

    def testSyntax(self):
        self.assertIs(self._root.syntax()[0], self._syntax[0])
        self.assertIs(self._root.syntax()[1], self._syntax[1])

    def testObjects(self):
        self.assertIs(self._root.objects()[0], self._objects[0])
        self.assertIs(self._root.objects()[1], self._objects[1])

    def testActions(self):
        self.assertIs(self._root.actions()[0], self._actions[0])
        self.assertIs(self._root.actions()[1], self._actions[1])

class TestObjectNodeBase(unittest.TestCase):

    def testProperties(self):
        source = os.path.abspath(os.path.join('..', '..', '..', 'framework', 'src', 'kernels', 'Diffusion.C'))
        include = os.path.abspath(os.path.join('..', '..', '..', 'framework', 'include', 'kernels', 'Diffusion.h'))
        kwargs = {'class':'Diffusion', 'label':'MooseApp', 'register_file':source, 'parameters':{'year':1980}}
        r = ObjectNodeBase(None, 'a', **kwargs)

        self.assertEqual(r.classname, 'Diffusion')
        self.assertEqual(r.group, 'MooseApp')
        self.assertEqual(r.source, source)
        self.assertEqual(r.header, include)
        self.assertEqual(r.parameters, dict(year=1980))
        self.assertEqual(r.markdown, 'kernels/Diffusion.md')

    def testHeader(self):
        r = ObjectNodeBase(None, 'a', header='foo')
        self.assertEqual(r.header, 'foo')

    def testMarkdown(self):
        r = ObjectNodeBase(None, 'a', header='/include/foo/bar.h')
        self.assertEqual(r.markdown, 'foo/bar.md')

        r = ObjectNodeBase(None, 'a', header='/include/foo/bar.h', markdown='andrew.md')
        self.assertEqual(r.markdown, 'andrew.md')

    def testErrors(self):
        with self.assertLogs(level='CRITICAL') as cm:
            r = ObjectNodeBase(None, 'a', source='')
        self.assertEqual(len(cm.output), 1)
        self.assertIn('MooseDocs requires', cm.output[0])

        with self.assertLogs(level='CRITICAL') as cm:
            r = ObjectNodeBase(None, 'a', source='wrong')
        self.assertEqual(len(cm.output), 1)
        self.assertIn('does not exist: wrong', cm.output[0])

class TestMooseObjectNode(unittest.TestCase):
    @mock.patch('mooseutils.colorText', side_effect=lambda t, c: '{}:{}'.format(c,t))
    def testRepr(self, *args):
        r = repr(MooseObjectNode(NodeBase(None, ''), 'a'))
        self.assertIn('LIGHT_YELLOW:a', r)

class TestActionNode(unittest.TestCase):
    def testProperties(self):
        r = ActionNode(None, '', tasks={'foo', 'bar'})
        self.assertEqual(r.tasks, {'foo', 'bar'})

    @mock.patch('mooseutils.colorText', side_effect=lambda t, c: '{}:{}'.format(c,t))
    def testRepr(self, *args):
        r = repr(ActionNode(NodeBase(None, ''), 'a'))
        self.assertIn('LIGHT_MAGENTA:a', r)

class TestMooseObjectActionNode(unittest.TestCase):
    @mock.patch('mooseutils.colorText', side_effect=lambda t, c: '{}:{}'.format(c,t))
    def testRepr(self, *args):
        r = repr(MooseObjectActionNode(NodeBase(None, ''), 'a'))
        self.assertIn('LIGHT_CYAN:a', r)


if __name__ == '__main__':
    unittest.main(verbosity=2)
