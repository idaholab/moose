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
from MooseDocs import common, base
from MooseDocs.common import exceptions
from MooseDocs.test import MooseDocsTestCase
from MooseDocs.extensions import core, command, floats, media
logging.basicConfig()

class TestImage(MooseDocsTestCase):
    EXTENSIONS = [core, command, floats, media]

    def setupContent(self):
        """Virtual method for populating Content section in configuration."""
        config = [dict(root_dir='large_media', content=['testing/Flag_of_Idaho.*'])]
        return common.get_content(config, '.md')


    def testAST(self):
        # no float
        ast = self.tokenize("!media Flag_of_Idaho.svg")
        self.assertSize(ast, 1)
        self.assertToken(ast(0), 'Image', src='Flag_of_Idaho.svg')

        # latex_src
        ast = self.tokenize("!media Flag_of_Idaho.svg latex_src=Flag_of_Idaho.pdf")
        self.assertSize(ast, 1)
        self.assertToken(ast(0), 'Image', src='Flag_of_Idaho.svg', tex='Flag_of_Idaho.pdf')

        # in float
        ast = self.tokenize("!media Flag_of_Idaho.svg caption=test id=idaho")
        self.assertSize(ast, 2)
        self.assertToken(ast(0), 'Float', bottom=True, command='figure')
        self.assertToken(ast(0,0), 'FloatCaption', key='idaho', prefix='Figure', number=1)
        self.assertToken(ast(0,0,0), 'Word', content='test')
        self.assertToken(ast(0,1), 'Image', src='Flag_of_Idaho.svg')
        self.assertToken(ast(1), 'Shortcut', key='idaho', link='#idaho')
        self.assertToken(ast(1,0), 'String', content='Figure 1')

        # in float
        ast = self.tokenize("!media Flag_of_Idaho.svg caption=test id=idaho prefix=Flag")
        self.assertSize(ast, 2)
        self.assertToken(ast(0), 'Float', bottom=True, command='figure')
        self.assertToken(ast(0,0), 'FloatCaption', key='idaho', prefix='Flag', number=1)
        self.assertToken(ast(0,0,0), 'Word', content='test')
        self.assertToken(ast(0,1), 'Image', src='Flag_of_Idaho.svg')
        self.assertToken(ast(1), 'Shortcut', key='idaho', link='#idaho')
        self.assertToken(ast(1,0), 'String', content='Flag 1')

    def testHTML(self):
        # no float
        _, res = self.execute("!media Flag_of_Idaho.svg")
        self.assertHTMLTag(res, 'body', size=1)
        self.assertHTMLTag(res(0), 'picture', size=1)
        self.assertHTMLTag(res(0,0), 'img', size=0, src='testing/Flag_of_Idaho.svg')

        # in float
        _, res = self.execute("!media Flag_of_Idaho.svg caption=test id=idaho")
        self.assertHTMLTag(res, 'body', size=1)
        self.assertHTMLTag(res(0), 'div', size=2, class_='moose-float-div', id_='idaho')
        self.assertHTMLTag(res(0,0), 'picture', size=1)
        self.assertHTMLTag(res(0,0,0), 'img', size=0, src='testing/Flag_of_Idaho.svg')
        self.assertHTMLTag(res(0,1), 'p', size=2, class_='moose-caption')
        self.assertHTMLTag(res(0,1,0), 'span', size=1, class_='moose-caption-heading', string='Figure 1: ')
        self.assertHTMLTag(res(0,1,1), 'span', size=1, class_='moose-caption-text', string='test')

    def testMaterialize(self):
        # no float
        ast = self.tokenize("!media Flag_of_Idaho.svg")
        res = self.render(ast, renderer=base.MaterializeRenderer())
        self.assertHTMLTag(res, 'div', size=1, class_='moose-content')
        self.assertHTMLTag(res(0), 'picture', size=1)
        self.assertHTMLTag(res(0,0), 'img', size=0, src='testing/Flag_of_Idaho.svg')

        # in float
        ast = self.tokenize("!media Flag_of_Idaho.svg caption=test id=idaho")
        res = self.render(ast, renderer=base.MaterializeRenderer())
        self.assertHTMLTag(res, 'div', size=1, class_='moose-content')
        self.assertHTMLTag(res(0), 'div', size=1, class_='card moose-float')
        self.assertHTMLTag(res(0,0), 'div', size=2, class_='card-content')
        self.assertHTMLTag(res(0,0,0), 'picture', size=1)
        self.assertHTMLTag(res(0,0,0,0), 'img', size=0, src='testing/Flag_of_Idaho.svg')
        self.assertHTMLTag(res(0,0,1), 'p', size=2, class_='moose-caption')
        self.assertHTMLTag(res(0,0,1,0), 'span', size=1, class_='moose-caption-heading', string='Figure 1: ')
        self.assertHTMLTag(res(0,0,1,1), 'span', size=1, class_='moose-caption-text', string='test')


    def testLatex(self):
        # wrong image format
        ast = self.tokenize("!media Flag_of_Idaho.svg")
        with self.assertLogs(level='ERROR') as cm:
            res = self.render(ast, renderer=base.LatexRenderer())
        self.assertEqual(len(cm.output), 1)
        self.assertIn("Images with the '.svg'", cm.output[0])

        # no float
        ast = self.tokenize("!media Flag_of_Idaho.svg latex_src=Flag_of_Idaho.pdf")
        res = self.render(ast, renderer=base.LatexRenderer())
        self.assertSize(res, 1)
        self.assertLatexCommand(res(0), 'includegraphics')
        self.assertIn('Flag_of_Idaho.pdf', res(0,0)['content'])

        # in float
        ast = self.tokenize("!media Flag_of_Idaho.svg latex_src=Flag_of_Idaho.pdf caption=test id=idaho")
        res = self.render(ast, renderer=base.LatexRenderer())
        self.assertSize(res, 1)
        self.assertLatexEnvironment(res(0), 'figure', size=2)
        self.assertLatexCommand(res(0,1), 'caption', size=2)
        self.assertLatexCommand(res(0,1,0), 'label', size=1, string='idaho')
        self.assertLatexString(res(0,1,1), 'test')
        self.assertLatexCommand(res(0,0), 'includegraphics')
        self.assertIn('Flag_of_Idaho.pdf', res(0,0,0)['content'])

class TestVideo(MooseDocsTestCase):
    EXTENSIONS = [core, command, floats, media]

    def setupContent(self):
        """Virtual method for populating Content section in configuration."""
        config = [dict(root_dir='large_media', content=['testing/*'])]
        return common.get_content(config, '.md')


    def testAST(self):
        # no float
        ast = self.tokenize("!media http://clips.vorwaerts-gmbh.de/VfE.webm")
        self.assertSize(ast, 1)
        self.assertToken(ast(0), 'Video', src='http://clips.vorwaerts-gmbh.de/VfE.webm')

        # poster
        ast = self.tokenize("!media http://clips.vorwaerts-gmbh.de/VfE.webm poster=big_buck_bunny.jpg")
        self.assertSize(ast, 1)
        self.assertToken(ast(0), 'Video', src='http://clips.vorwaerts-gmbh.de/VfE.webm', poster='big_buck_bunny.jpg')

        # latex_src
        ast = self.tokenize("!media http://clips.vorwaerts-gmbh.de/VfE.webm latex_src=Flag_of_Idaho.pdf")
        self.assertSize(ast, 1)
        self.assertToken(ast(0), 'Video', src='http://clips.vorwaerts-gmbh.de/VfE.webm', tex='Flag_of_Idaho.pdf')

        # in float
        ast = self.tokenize("!media http://clips.vorwaerts-gmbh.de/VfE.webm caption=test id=idaho")
        self.assertSize(ast, 2)
        self.assertToken(ast(0), 'Float', bottom=True, command='figure')
        self.assertToken(ast(0,0), 'FloatCaption', key='idaho', prefix='Figure', number=1)
        self.assertToken(ast(0,0,0), 'Word', content='test')
        self.assertToken(ast(0,1), 'Video', src='http://clips.vorwaerts-gmbh.de/VfE.webm')
        self.assertToken(ast(1), 'Shortcut', key='idaho', link='#idaho')
        self.assertToken(ast(1,0), 'String', content='Figure 1')

        # in float
        ast = self.tokenize("!media http://clips.vorwaerts-gmbh.de/VfE.webm caption=test id=idaho prefix=Flag")
        self.assertSize(ast, 2)
        self.assertToken(ast(0), 'Float', bottom=True, command='figure')
        self.assertToken(ast(0,0), 'FloatCaption', key='idaho', prefix='Flag', number=1)
        self.assertToken(ast(0,0,0), 'Word', content='test')
        self.assertToken(ast(0,1), 'Video', src='http://clips.vorwaerts-gmbh.de/VfE.webm')
        self.assertToken(ast(1), 'Shortcut', key='idaho', link='#idaho')
        self.assertToken(ast(1,0), 'String', content='Flag 1')

    def testHTML(self):
        # no float with poster
        _, res = self.execute("!media http://clips.vorwaerts-gmbh.de/VfE.webm poster=big_buck_bunny.jpg")
        self.assertHTMLTag(res, 'body', size=1)
        self.assertHTMLTag(res(0), 'div', size=1, class_='moose-video-div')
        self.assertHTMLTag(res(0,0), 'video', size=1, class_='moose-video', width='100%',
                           controls=True, poster='/testing/big_buck_bunny.jpg')
        self.assertHTMLTag(res(0,0,0), 'source', src="http://clips.vorwaerts-gmbh.de/VfE.webm")

        # in float
        _, res = self.execute("!media http://clips.vorwaerts-gmbh.de/VfE.webm caption=test id=idaho")
        self.assertHTMLTag(res, 'body', size=1)
        self.assertHTMLTag(res(0), 'div', size=2, class_='moose-float-div', id_='idaho')
        self.assertHTMLTag(res(0,0), 'div', size=1, class_='moose-video-div')
        self.assertHTMLTag(res(0,0,0), 'video', size=1, class_='moose-video', width='100%', controls=True)
        self.assertHTMLTag(res(0,0,0,0), 'source', src="http://clips.vorwaerts-gmbh.de/VfE.webm")
        self.assertHTMLTag(res(0,1), 'p', size=2, class_='moose-caption')
        self.assertHTMLTag(res(0,1,0), 'span', size=1, class_='moose-caption-heading', string='Figure 1: ')
        self.assertHTMLTag(res(0,1,1), 'span', size=1, class_='moose-caption-text', string='test')

    def testMaterialize(self):
        # no float with poster
        ast = self.tokenize("!media http://clips.vorwaerts-gmbh.de/VfE.webm poster=big_buck_bunny.jpg")
        res = self.render(ast, renderer=base.MaterializeRenderer())
        self.assertHTMLTag(res, 'div', size=1, class_='moose-content')
        self.assertHTMLTag(res(0), 'div', size=1, class_='moose-video-div')
        self.assertHTMLTag(res(0,0), 'video', size=1, class_='moose-video', width='100%',
                           controls=True, poster='/testing/big_buck_bunny.jpg')
        self.assertHTMLTag(res(0,0,0), 'source', src="http://clips.vorwaerts-gmbh.de/VfE.webm")

        # in float
        ast = self.tokenize("!media http://clips.vorwaerts-gmbh.de/VfE.webm caption=test id=idaho")
        res = self.render(ast, renderer=base.MaterializeRenderer())
        self.assertHTMLTag(res, 'div', size=1, class_='moose-content')
        self.assertHTMLTag(res(0), 'div', size=1, class_='card moose-float', id_='idaho')
        self.assertHTMLTag(res(0,0), 'div', size=2, class_='card-image')
        self.assertHTMLTag(res(0,0,0), 'div', size=1, class_='moose-video-div')
        self.assertHTMLTag(res(0,0,0,0), 'video', size=1, class_='moose-video', width='100%', controls=True)
        self.assertHTMLTag(res(0,0,0,0,0), 'source', src="http://clips.vorwaerts-gmbh.de/VfE.webm")
        self.assertHTMLTag(res(0,0,1), 'p', size=2, class_='moose-caption')
        self.assertHTMLTag(res(0,0,1,0), 'span', size=1, class_='moose-caption-heading', string='Figure 1: ')
        self.assertHTMLTag(res(0,0,1,1), 'span', size=1, class_='moose-caption-text', string='test')


    def testLatex(self):
        # wrong image format
        ast = self.tokenize("!media http://clips.vorwaerts-gmbh.de/VfE.webm")
        with self.assertLogs(level='ERROR') as cm:
            res = self.render(ast, renderer=base.LatexRenderer())
        self.assertEqual(len(cm.output), 1)
        self.assertIn("are not supported with LaTeX", cm.output[0])

        # no float
        ast = self.tokenize("!media http://clips.vorwaerts-gmbh.de/VfE.webm latex_src=Flag_of_Idaho.pdf")
        res = self.render(ast, renderer=base.LatexRenderer())
        self.assertSize(res, 4)
        self.assertLatexCommand(res(0), 'includegraphics')
        self.assertIn('Flag_of_Idaho.pdf', res(0,0)['content'])
        self.assertLatexCommand(res(2), 'url', string='http://clips.vorwaerts-gmbh.de/VfE.webm')

        # in float
        ast = self.tokenize("!media http://clips.vorwaerts-gmbh.de/VfE.webm latex_src=Flag_of_Idaho.pdf caption=test id=idaho")
        res = self.render(ast, renderer=base.LatexRenderer())
        self.assertSize(res, 1)
        self.assertLatexEnvironment(res(0), 'figure', size=5)
        self.assertLatexCommand(res(0,4), 'caption', size=2)
        self.assertLatexCommand(res(0,4,0), 'label', size=1, string='idaho')
        self.assertLatexString(res(0,4,1), 'test')
        self.assertLatexCommand(res(0,0), 'includegraphics')
        self.assertIn('Flag_of_Idaho.pdf', res(0,0,0)['content'])
        self.assertLatexCommand(res(0,2), 'url', string='http://clips.vorwaerts-gmbh.de/VfE.webm')


class TestYouTube(MooseDocsTestCase):
    EXTENSIONS = [core, command, floats, media]

    def setupContent(self):
        """Virtual method for populating Content section in configuration."""
        config = [dict(root_dir='large_media', content=['testing/Flag_of_Idaho.*'])]
        return common.get_content(config, '.md')

    def testAST(self):
        ast = self.tokenize("!media https://www.youtube.com/not_real")
        self.assertSize(ast, 1)
        self.assertToken(ast(0), 'Video', src='https://www.youtube.com/not_real', youtube=True)

    def testHTML(self):
        _, res = self.execute("!media https://www.youtube.com/not_real")
        self.assertHTMLTag(res, 'body', size=1)
        self.assertHTMLTag(res(0), 'div', size=1)
        self.assertHTMLTag(res(0,0), 'iframe', src="https://www.youtube.com/not_real")

class TestFloatReference(MooseDocsTestCase):
    EXTENSIONS = [core, command, floats]

    def testLocalReference(self):
        ast = self.tokenize('[!ref](dummy)')
        self.assertSize(ast, 1)
        self.assertToken(ast(0), 'Paragraph', size=1)
        self.assertToken(ast(0,0), 'FloatReference', size=0, label='dummy', filename=None)

    def testNonLocalReference(self):
        ast = self.tokenize('[!ref](foo_bar.md#dummy)')
        self.assertSize(ast, 1)
        self.assertToken(ast(0), 'Paragraph', size=1)
        self.assertToken(ast(0,0), 'FloatReference', size=0, label='dummy', filename='foo_bar.md')

if __name__ == '__main__':
    unittest.main(verbosity=2)
