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
from MooseDocs.extensions import core, modal
from MooseDocs import base
logging.basicConfig()

class TestModalLink(MooseDocsTestCase):
    EXTENSIONS = [core, modal]

    def testHTML(self):
        ast = modal.ModalLink(None, string='test')
        res = self.render(ast, renderer=base.HTMLRenderer())
        self.assertHTMLTag(res, 'body', size=1)
        self.assertHTMLTag(res(0), 'span', size=1, class_='moose-modal-link', string='test')

    def testHTMLErrors(self):
        ast = modal.ModalLink(None)

        with self.assertLogs(level=logging.ERROR) as cm:
            res = self.render(ast, renderer=base.HTMLRenderer())
        self.assertEqual(len(cm.output), 1)
        self.assertIn("The \'ModalLink\' token requires children", cm.output[0])

    def testMaterialize(self):

        # Content as Token
        ast = modal.ModalLink(None, string='test', content=core.Code(None, content='1+2=3'))
        res = self.render(ast, renderer=base.MaterializeRenderer())
        self.assertSize(res, 2)

        uid = res(1)['id']
        self.assertHTMLTag(res(0), 'a', size=1, href='#{}'.format(uid), string='test')
        self.assertHTMLTag(res(1), 'div', size=1, class_='moose-modal modal', id_=uid)
        self.assertHTMLTag(res(1,0), 'div', size=1, class_='modal-content')
        self.assertHTMLTag(res(1,0,0), 'pre', size=1, class_='moose-pre')
        self.assertHTMLTag(res(1,0,0,0), 'code', size=1, class_='language-text', string='1+2=3')

        # Content as String
        ast = modal.ModalLink(None, string='test', content='1+2=3')
        res = self.render(ast, renderer=base.MaterializeRenderer())
        self.assertSize(res, 2)

        uid = res(1)['id']
        self.assertHTMLTag(res(0), 'a', size=1, href='#{}'.format(uid), string='test')
        self.assertHTMLTag(res(1), 'div', size=1, class_='moose-modal modal', id_=uid)
        self.assertHTMLTag(res(1,0), 'div', size=1, class_='modal-content')
        self.assertHTMLTag(res(1,0,0), 'p', size=1, string='1+2=3')

        # title
        ast = modal.ModalLink(None, string='test', content='1+2=3', title='math')
        res = self.render(ast, renderer=base.MaterializeRenderer())
        self.assertSize(res, 2)

        uid = res(1)['id']
        self.assertHTMLTag(res(0), 'a', size=1, href='#{}'.format(uid), string='test')
        self.assertHTMLTag(res(1), 'div', size=1, class_='moose-modal modal', id_=uid)
        self.assertHTMLTag(res(1,0), 'div', size=2, class_='modal-content')
        self.assertHTMLTag(res(1,0,0), 'h4', size=1, string='math')
        self.assertHTMLTag(res(1,0,1), 'p', size=1, string='1+2=3')

    def testMaterializeErrors(self):

        ast = modal.ModalLink(None, content=core.Code(None, content='1+2=3'))
        with self.assertLogs(level=logging.ERROR) as cm:
            res = self.render(ast, renderer=base.MaterializeRenderer())
        self.assertEqual(len(cm.output), 1)
        self.assertIn("The \'ModalLink\' token requires children", cm.output[0])

        ast = modal.ModalLink(None, string='test', content=1980)
        with self.assertLogs(level=logging.ERROR) as cm:
            res = self.render(ast, renderer=base.MaterializeRenderer())
        self.assertEqual(len(cm.output), 1)
        self.assertIn("The \'ModalLink\' token 'content' attribute must be a string or Token", cm.output[0])

        ast = modal.ModalLink(None, string='test')
        with self.assertLogs(level=logging.ERROR) as cm:
            res = self.render(ast, renderer=base.MaterializeRenderer())
        self.assertEqual(len(cm.output), 1)
        self.assertIn("The \'ModalLink\' token 'content' attribute must be a string or Token", cm.output[0])

    def testLatex(self):
        ast = modal.ModalLink(None, string='test')
        res = self.render(ast, renderer=base.LatexRenderer())
        self.assertSize(res, 1)
        self.assertLatexString(res(0), content='test')

class TestModalSourceLink(MooseDocsTestCase):
    EXTENSIONS = [core, modal]

    def testHTML(self):
        ast = modal.ModalSourceLink(None, src='test')
        res = self.render(ast, renderer=base.HTMLRenderer())
        self.assertHTMLTag(res, 'body', size=1)
        self.assertHTMLTag(res(0), 'span', size=1, class_='moose-source-filename', string='(python/MooseDocs/test/extensions/test)')

        ast = modal.ModalSourceLink(None, src='test', string='string')

        res = self.render(ast, renderer=base.HTMLRenderer())
        self.assertHTMLTag(res, 'body', size=1)
        self.assertHTMLTag(res(0), 'span', size=1, class_='moose-source-filename', string='string')

        # link_prefix
        ast = modal.ModalSourceLink(None, src='test', link_prefix='Foo:')
        res = self.render(ast, renderer=base.HTMLRenderer())
        self.assertHTMLTag(res, 'body', size=1)
        self.assertHTMLTag(res(0), 'span', size=1, class_='moose-source-filename', string='(Foo: python/MooseDocs/test/extensions/test)')

    def testMaterialize(self):

        # 'src' with source enabled
        ast = modal.ModalSourceLink(None, src='framework/Makefile')
        res = self.render(ast, renderer=base.MaterializeRenderer())

        self.assertSize(res, 2)
        uid = res(1)['id']
        self.assertHTMLTag(res(0), 'a', size=1, href='#{}'.format(uid),
                           string='(python/MooseDocs/test/extensions/framework/Makefile)',
                           class_='moose-source-filename tooltipped modal-trigger')
        self.assertHTMLTag(res(1), 'div', size=2, class_='moose-modal modal', id_=uid)
        self.assertHTMLTag(res(1,0), 'div', size=2, class_='modal-content')
        self.assertHTMLTag(res(1,0,0), 'h4', size=1, string='(python/MooseDocs/test/extensions/framework/Makefile)')
        self.assertHTMLTag(res(1,0,1), 'pre', size=1, class_='moose-pre')
        self.assertHTMLTag(res(1,0,1,0), 'code', size=1, class_='language-text')

        self.assertIn('MOOSE Application Standard Makefile', res(1,0,1,0,0)['content'])
        self.assertHTMLTag(res(1,1), 'div', size=1, class_='modal-footer')
        self.assertHTMLTag(res(1,1,0), 'a', size=1, class_='modal-close btn-flat', string='Close')

        # title
        ast = modal.ModalSourceLink(None, src='framework/Makefile', title='Makefile')
        res = self.render(ast, renderer=base.MaterializeRenderer())
        self.assertSize(res, 2)
        uid = res(1)['id']
        self.assertHTMLTag(res(0), 'a', size=1, href='#{}'.format(uid),
                           string='(python/MooseDocs/test/extensions/framework/Makefile)',
                           class_='moose-source-filename tooltipped modal-trigger')
        self.assertHTMLTag(res(1), 'div', size=2, class_='moose-modal modal', id_=uid)
        self.assertHTMLTag(res(1,0), 'div', size=2, class_='modal-content')
        self.assertHTMLTag(res(1,0,0), 'h4', size=1, string='Makefile')
        self.assertHTMLTag(res(1,0,1), 'pre', size=1, class_='moose-pre')
        self.assertHTMLTag(res(1,0,1,0), 'code', size=1, class_='language-text')

        self.assertIn('MOOSE Application Standard Makefile', res(1,0,1,0,0)['content'])
        self.assertHTMLTag(res(1,1), 'div', size=1, class_='modal-footer')
        self.assertHTMLTag(res(1,1,0), 'a', size=1, class_='modal-close btn-flat', string='Close')

        # with children
        ast = modal.ModalSourceLink(None, src='framework/Makefile', string='Makefile')
        res = self.render(ast, renderer=base.MaterializeRenderer())
        self.assertSize(res, 2)
        uid = res(1)['id']
        self.assertHTMLTag(res(0), 'a', size=1, href='#{}'.format(uid),
                           string='Makefile',
                           class_='moose-source-filename tooltipped modal-trigger')
        self.assertHTMLTag(res(1), 'div', size=2, class_='moose-modal modal', id_=uid)
        self.assertHTMLTag(res(1,0), 'div', size=2, class_='modal-content')
        self.assertHTMLTag(res(1,0,0), 'h4', size=1, string='(python/MooseDocs/test/extensions/framework/Makefile)')
        self.assertHTMLTag(res(1,0,1), 'pre', size=1, class_='moose-pre')
        self.assertHTMLTag(res(1,0,1,0), 'code', size=1, class_='language-text')

        self.assertIn('MOOSE Application Standard Makefile', res(1,0,1,0,0)['content'])
        self.assertHTMLTag(res(1,1), 'div', size=1, class_='modal-footer')
        self.assertHTMLTag(res(1,1,0), 'a', size=1, class_='modal-close btn-flat', string='Close')

        # with language
        ast = modal.ModalSourceLink(None, src='framework/Makefile', language='other')
        res = self.render(ast, renderer=base.MaterializeRenderer())
        self.assertHTMLTag(res(1,0,1,0), 'code', size=1, class_='language-other')

        # link_prefix
        ast = modal.ModalSourceLink(None, src='framework/Makefile', link_prefix='Foo:')
        res = self.render(ast, renderer=base.MaterializeRenderer())
        self.assertHTMLTag(res(0), 'a', string='(Foo: python/MooseDocs/test/extensions/framework/Makefile)')

    def testMaterializeErrors(self):

        ast = modal.ModalSourceLink(None, src='Makefile')
        with self.assertLogs(level=logging.ERROR) as cm:
            res = self.render(ast, renderer=base.MaterializeRenderer())
        self.assertEqual(len(cm.output), 1)
        self.assertIn("Multiple files match the supplied filename", cm.output[0])

        ast = modal.ModalSourceLink(None, src='wrong')
        with self.assertLogs(level=logging.ERROR) as cm:
            res = self.render(ast, renderer=base.MaterializeRenderer())
        self.assertEqual(len(cm.output), 1)
        self.assertIn("Unable to locate file that matches the supplied filename", cm.output[0])

class TestModalSourceLinkDisableSource(MooseDocsTestCase):
    EXTENSIONS = [core, modal]

    def setupExtension(self, ext):
        if ext == modal:
            return dict(hide_source=True, exceptions=['*.i'])

    def testMaterialize(self):
        ast = modal.ModalSourceLink(None, src='framework/Makefile')
        res = self.render(ast, renderer=base.MaterializeRenderer())
        self.assertSize(res, 1)
        self.assertHTMLTag(res(0), 'span', size=1,
                           string='(python/MooseDocs/test/extensions/framework/Makefile)',
                           class_='moose-source-filename tooltipped')

        # An input file should render because it matches a pattern in the 'exceptions' setting
        ast = modal.ModalSourceLink(None, src='moose/test/tests/kernels/simple_diffusion/simple_diffusion.i')
        res = self.render(ast, renderer=base.MaterializeRenderer())
        self.assertSize(res, 2)
        self.assertHTMLTag(res(0), 'a', size=1,
                           string='(python/MooseDocs/test/extensions/moose/test/tests/kernels/simple_diffusion/simple_diffusion.i)',
                           class_='moose-source-filename tooltipped modal-trigger')
        self.assertHTMLTag(res(1), 'div', size=2, class_='moose-modal modal')

    def testLatex(self):
        ast = modal.ModalSourceLink(None, src='framework/Makefile')
        res = self.render(ast, renderer=base.LatexRenderer())
        self.assertSize(res, 1)
        self.assertLatexString(res(0), content='(python/MooseDocs/test/extensions/framework/Makefile)')

        ast = modal.ModalSourceLink(None, string='test',
                                    src='moose/test/tests/kernels/simple_diffusion/simple_diffusion.i')
        res = self.render(ast, renderer=base.LatexRenderer())
        self.assertSize(res, 1)
        self.assertLatexString(res(0), content='test')

        print(res, "\n")


if __name__ == '__main__':
    unittest.main(verbosity=2)
