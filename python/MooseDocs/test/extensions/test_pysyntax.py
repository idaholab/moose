#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import unittest
import logging
from MooseDocs.test import MooseDocsTestCase
from MooseDocs.extensions import core, command, pysyntax
from MooseDocs import base
logging.basicConfig()

class TestObj(object):
    """class doc"""
    def __init__(self, data):
        pass

    @property
    def __internal_prop__(self):
        """internal property doc"""
        pass
    @property
    def public_prop(self):
        """public property doc"""
        pass
    @property
    def _protected_prop(self):
        """protected property doc"""
        pass
    @property
    def __private_prop(self):
        """private property doc"""
        pass
    def publicMethod(self, arg):
        """public method doc"""
        pass
    def _protectedMethod(self, arg):
        """protected method doc"""
        pass
    def __privateMethod(self, arg):
        """private method doc"""
        pass
    def __internalMethod__(self, arg):
        """internal method doc"""
        pass

class TestPySyntax(unittest.TestCase):
    def testInfo(self):
        info = pysyntax.PySyntax.Info('public_prop', TestObj.public_prop)
        self.assertEqual(info.internal, False)
        self.assertEqual(info.private, False)
        self.assertEqual(info.protected, False)
        self.assertEqual(info.public, True)
        self.assertEqual(info.function, False)
        self.assertEqual(info.signature, None)
        self.assertEqual(info.documentation, "public property doc")

        info = pysyntax.PySyntax.Info('_protected_prop', TestObj._protected_prop)
        self.assertEqual(info.internal, False)
        self.assertEqual(info.private, False)
        self.assertEqual(info.protected, True)
        self.assertEqual(info.public, False)
        self.assertEqual(info.function, False)
        self.assertEqual(info.signature, None)
        self.assertEqual(info.documentation, "protected property doc")

        info = pysyntax.PySyntax.Info('_TestObj__private_prop', TestObj._TestObj__private_prop)
        self.assertEqual(info.internal, False)
        self.assertEqual(info.private, True)
        self.assertEqual(info.protected, False)
        self.assertEqual(info.public, False)
        self.assertEqual(info.function, False)
        self.assertEqual(info.signature, None)
        self.assertEqual(info.documentation, "private property doc")

        info = pysyntax.PySyntax.Info('__internal_prop__', TestObj.__internal_prop__)
        self.assertEqual(info.internal, True)
        self.assertEqual(info.private, False)
        self.assertEqual(info.protected, False)
        self.assertEqual(info.public, False)
        self.assertEqual(info.function, False)
        self.assertEqual(info.signature, None)
        self.assertEqual(info.documentation, "internal property doc")

        info = pysyntax.PySyntax.Info('publicMethod', TestObj.publicMethod)
        self.assertEqual(info.internal, False)
        self.assertEqual(info.private, False)
        self.assertEqual(info.protected, False)
        self.assertEqual(info.public, True)
        self.assertEqual(info.function, True)
        self.assertEqual(info.signature, '(arg)')
        self.assertEqual(info.documentation, "public method doc")

        info = pysyntax.PySyntax.Info('_protectedMethod', TestObj._protectedMethod)
        self.assertEqual(info.internal, False)
        self.assertEqual(info.private, False)
        self.assertEqual(info.protected, True)
        self.assertEqual(info.public, False)
        self.assertEqual(info.function, True)
        self.assertEqual(info.signature, '(arg)')
        self.assertEqual(info.documentation, "protected method doc")

        info = pysyntax.PySyntax.Info('_TestObj__privateMethod', TestObj._TestObj__privateMethod)
        self.assertEqual(info.internal, False)
        self.assertEqual(info.private, True)
        self.assertEqual(info.protected, False)
        self.assertEqual(info.public, False)
        self.assertEqual(info.function, True)
        self.assertEqual(info.signature, '(arg)')
        self.assertEqual(info.documentation, "private method doc")

        info = pysyntax.PySyntax.Info('__internalMethod__', TestObj.__internalMethod__)
        self.assertEqual(info.internal, True)
        self.assertEqual(info.private, False)
        self.assertEqual(info.protected, False)
        self.assertEqual(info.public, False)
        self.assertEqual(info.function, True)
        self.assertEqual(info.signature, '(arg)')
        self.assertEqual(info.documentation, "internal method doc")

    def testClass(self):
        Info = pysyntax.PySyntax.Info
        doc = pysyntax.PySyntax(TestObj)
        self.assertEqual(doc.documentation, 'class doc')
        self.assertEqual(doc.filename, __file__)
        self.assertEqual(doc.signature, '(data)')

        public = [('publicMethod', Info('publicMethod', TestObj.publicMethod)),
                  ('public_prop', Info('public_prop', TestObj.public_prop))]
        protected = [('_protectedMethod', Info('_protectedMethod', TestObj._protectedMethod)),
                     ('_protected_prop', Info('_protected_prop', TestObj._protected_prop))]
        private = [('_TestObj__privateMethod', Info('_TestObj__privateMethod', TestObj._TestObj__privateMethod)),
                   ('_TestObj__private_prop', Info('_TestObj__private_prop', TestObj._TestObj__private_prop))]
        internal = [('__internalMethod__', Info('__internalMethod__', TestObj.__internalMethod__)),
                    ('__internal_prop__', Info('__internal_prop__', TestObj.__internal_prop__))]

        mem = [(k,v) for k, v in doc.items(public=True)]
        self.assertEqual(mem, public)

        mem = [(k,v) for k, v in doc.items(protected=True)]
        self.assertEqual(mem, protected)

        mem = [(k,v) for k, v in doc.items(private=True)]
        self.assertEqual(mem, private)

        mem = [(k,v) for k, v in doc.items(internal=True)]
        self.assertIn(internal[0], mem)
        self.assertIn(internal[1], mem)

        mem = [(k,v) for k, v in doc.items(function=True)]
        self.assertIn(public[0], mem)
        self.assertNotIn(public[1], mem)
        self.assertIn(protected[0], mem)
        self.assertNotIn(protected[1], mem)
        self.assertIn(private[0], mem)
        self.assertNotIn(private[1], mem)
        self.assertIn(internal[0], mem)
        self.assertNotIn(internal[1], mem)

        mem = [(k,v) for k, v in doc.items(public=True, protected=True)]
        self.assertEqual(mem, protected + public)

        mem = [(k,v) for k, v in doc.items(public=True, private=True)]
        self.assertEqual(mem, private + public)

        mem = [(k,v) for k, v in doc.items(public=True, function=True)]
        self.assertEqual(mem, public[:1])

class TestPySyntaxExtension(MooseDocsTestCase):
    EXTENSIONS = [core, command, pysyntax]

    def testClassCommandAST(self):
        ast = self.tokenize('!pysyntax class name=MooseDocs.extensions.pysyntax.PySyntaxExtension')
        self.assertToken(ast(0), 'PyClass')
        self.assertToken(ast(0,0), 'Heading', level=2, class_='moose-pysyntax-class-heading', string='MooseDocs.extensions.pysyntax.PySyntaxExtension')
        self.assertToken(ast(0,1), 'Monospace', string='MooseDocs.extensions.pysyntax.PySyntaxExtension(**kwargs)')
        self.assertToken(ast(0,2), 'Paragraph', size=14)
        self.assertToken(ast(0,2,0), 'Word', content='Extension')
        self.assertToken(ast(0,3), 'Heading', level=3, class_='moose-pysyntax-member-heading')
        self.assertToken(ast(0,3,0), 'Strong')
        self.assertToken(ast(0,3,0,0), 'Monospace', string='EXTENSION_COMMANDS')
        self.assertToken(ast(0,4), 'Paragraph')

    def testFunctionCommandAST(self):
        ast = self.tokenize('!pysyntax function name=MooseDocs.extensions.pysyntax.make_extension')
        self.assertToken(ast(0), 'PyFunction')
        self.assertToken(ast(0,0), 'Heading', level=2, class_='moose-pysyntax-member-heading')
        self.assertToken(ast(0,0,0), 'Strong')
        self.assertToken(ast(0,0,0,0), 'Monospace', string='make_extension(**kwargs)')

if __name__ == '__main__':
    unittest.main(verbosity=2)
