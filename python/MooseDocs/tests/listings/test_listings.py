#!/usr/bin/env python
#pylint: disable=missing-docstring
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import unittest
from MooseDocs.testing import MarkdownTestCase

class TestListings(MarkdownTestCase):
    EXTENSIONS = ['MooseDocs.extensions.listings', 'MooseDocs.extensions.refs', 'extra']

    def testDefault(self):
        md = '!listing test/tests/kernels/simple_diffusion/simple_diffusion.i'
        self.assertConvert('testDefault.html', md)

    def testDefaultNoButton(self):
        md = '!listing test/tests/kernels/simple_diffusion/simple_diffusion.i'
        self.assertConvert('testDefaultNoButton.html', md)

    def testStart(self):
        md = '!listing test/tests/kernels/simple_diffusion/simple_diffusion.i start=[Outputs]'
        self.assertConvert('testStart.html', md)

    def testEnd(self):
        md = '!listing test/tests/kernels/simple_diffusion/simple_diffusion.i end=[Variables]'
        self.assertConvert('testEnd.html', md)

    def testStartEnd(self):
        md = '!listing test/tests/kernels/simple_diffusion/simple_diffusion.i start=[Variables] ' \
             'end=[BCs]'
        self.assertConvert('testStartEnd.html', md)

    def testStartEndIncludeEnd(self):
        md = '!listing test/tests/kernels/simple_diffusion/simple_diffusion.i start=[Variables] ' \
             'end=[BCs] include-end=True'
        self.assertConvert('testStartEndIncludeEnd.html', md)

    def testLine(self):
        md = '!listing test/tests/kernels/simple_diffusion/simple_diffusion.i line=ernel'
        self.assertConvert('testLine.html', md)

    def testContentError(self):
        md = '!listing test/tests/kernels/simple_diffusion/simple_diffusion.i line=notfound'
        self.assertConvert('testContentError.html', md)

    def testFileError(self):
        md = '!listing test/tests/kernels/simple_diffusion/not_a_file.txt'
        self.assertConvert('testFileError.html', md)

    def testCaption(self):
        md = '!listing test/tests/kernels/simple_diffusion/simple_diffusion.i start=[Outputs] ' \
             'caption=Outputs Block'
        self.assertConvert('testCaption.html', md)

    def testCaptionId(self):
        md = '!listing test/tests/kernels/simple_diffusion/simple_diffusion.i start=[Outputs] ' \
             'id=foo caption=Outputs Block'
        self.assertConvert('testCaptionId.html', md)

    def testStripHeader(self):
        md = '!listing framework/src/kernels/Diffusion.C end=template'
        self.assertConvert('testStripHeader.html', md)

    def testDisableStripHeader(self):
        md = '!listing framework/src/kernels/Diffusion.C end=template strip-header=0'
        self.assertConvert('testDisableStripHeader.html', md)

    def testLanguage(self):
        md = '!listing framework/src/kernels/Diffusion.C end=template language=foo'
        self.assertConvert('testLanguage.html', md)

    def testStripExtraNewLines(self):
        md = '!listing framework/src/kernels/Diffusion.C end=Diffusion::Diffusion ' \
             'strip-extra-newlines=true'
        self.assertConvert('testStripExtraNewLines.html', md)

    def testDisableStripExtraNewLines(self):
        md = '!listing framework/src/kernels/Diffusion.C end=Diffusion::Diffusion ' \
             'strip-extra-newlines=false'
        self.assertConvert('testDisableStripExtraNewLines.html', md)

    def testStripLeadingWhitespace(self):
        md = '!listing framework/src/kernels/Diffusion.C line=Kernel(parameters) ' \
             'strip-leading-whitespace=1'
        self.assertConvert('testStripLeadingWhitespace.html', md)

    def testDisableStripLeadingWhitespace(self): #pylint: disable=invalid-name
        md = '!listing framework/src/kernels/Diffusion.C line=Kernel(parameters) ' \
             'strip-leading-whitespace=false'
        self.assertConvert('testDisableStripLeadingWhitespace.html', md)

    def testPrefixSuffix(self):
        md = '!listing framework/src/kernels/Diffusion.C line=Kernel(parameters) ' \
             'strip-leading-whitespace=1 prefix=BEFORE suffix=AFTER'
        self.assertConvert('testPrefixSuffix.html', md)

    def testIndent(self):
        md = '!listing framework/src/kernels/Diffusion.C line=Kernel(parameters) indent=8 ' \
             'strip-leading-whitespace=1'
        self.assertConvert('testIndent.html', md)

    def testInputListing(self):
        md = '!listing test/tests/kernels/simple_diffusion/simple_diffusion.i block=Kernels ' \
             'id=diffusion_block caption=Diffusion Kernel Input Syntax'
        self.assertConvert('testInputListing.html', md)

    def testRef(self):
        md = '!listing test/tests/kernels/simple_diffusion/simple_diffusion.i start=[Outputs] ' \
             'id=foo\n\n\\ref{foo}'
        self.assertConvert('testRef.html', md)

    def testFenced(self):
        md = 'First fence without listing.\n\n```python\nx+y=1;\n```\n\n'
        md += 'Second fence without listing.\n\n```python\nx+z=2;\n```\n\n'
        md += 'Third fence.\n\n!listing caption=Third id=three\n```\nz+y=3;\n```\n\n'
        self.assertConvert('testFenced.html', md)

    def testListingTick(self):
        md = 'This should (`!listing`) be a paragraph.'
        self.assertConvert('testListingTick.html', md)

    def testListingMainComment(self):
        md = '!listing modules/phase_field/test/tests/KKS_system/kks_example.i main_comment=True'
        self.assertConvert('testListingMainComment.html', md)

if __name__ == '__main__':
    unittest.main(verbosity=2)
