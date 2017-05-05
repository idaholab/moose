#!/usr/bin/env python
#pylint: disable=missing-docstring
####################################################################################################
#                                    DO NOT MODIFY THIS HEADER                                     #
#                   MOOSE - Multiphysics Object Oriented Simulation Environment                    #
#                                                                                                  #
#                              (c) 2010 Battelle Energy Alliance, LLC                              #
#                                       ALL RIGHTS RESERVED                                        #
#                                                                                                  #
#                            Prepared by Battelle Energy Alliance, LLC                             #
#                               Under Contract No. DE-AC07-05ID14517                               #
#                               With the U. S. Department of Energy                                #
#                                                                                                  #
#                               See COPYRIGHT for full restrictions                                #
####################################################################################################

import unittest
from MooseDocs.testing import MarkdownTestCase

class TestListings(MarkdownTestCase):
    EXTENSIONS = ['MooseDocs.extensions.listings', 'MooseDocs.extensions.refs', 'extra']

    def testDefault(self):
        md = '!listing test/tests/kernels/simple_diffusion/simple_diffusion.i'
        self.assertConvert('testDefault.html', md)

    def testDefaultNoButton(self):
        md = '!listing test/tests/kernels/simple_diffusion/simple_diffusion.i copy-button=0'
        self.assertConvert('testDefaultNoButton.html', md)

    def testStart(self):
        md = '!listing test/tests/kernels/simple_diffusion/simple_diffusion.i start=[Outputs] ' \
             'copy-button=0'
        self.assertConvert('testStart.html', md)

    def testEnd(self):
        md = '!listing test/tests/kernels/simple_diffusion/simple_diffusion.i end=[Variables] ' \
             'copy-button=0'
        self.assertConvert('testEnd.html', md)

    def testStartEnd(self):
        md = '!listing test/tests/kernels/simple_diffusion/simple_diffusion.i start=[Variables] ' \
             'end=[BCs] copy-button=0'
        self.assertConvert('testStartEnd.html', md)

    def testStartEndIncludeEnd(self):
        md = '!listing test/tests/kernels/simple_diffusion/simple_diffusion.i start=[Variables] ' \
             'end=[BCs] include-end=True  copy-button=0'
        self.assertConvert('testStartEndIncludeEnd.html', md)

    def testLine(self):
        md = '!listing test/tests/kernels/simple_diffusion/simple_diffusion.i line=ernel ' \
             'copy-button=0'
        self.assertConvert('testLine.html', md)

    def testContentError(self):
        md = '!listing test/tests/kernels/simple_diffusion/simple_diffusion.i line=notfound ' \
             'copy-button=0'
        self.assertConvert('testContentError.html', md)

    def testFileError(self):
        md = '!listing test/tests/kernels/simple_diffusion/not_a_file.txt copy-button=0'
        self.assertConvert('testFileError.html', md)

    def testCaption(self):
        md = '!listing test/tests/kernels/simple_diffusion/simple_diffusion.i start=[Outputs] ' \
             'caption=Outputs Block copy-button=0'
        self.assertConvert('testCaption.html', md)

    def testCaptionId(self):
        md = '!listing test/tests/kernels/simple_diffusion/simple_diffusion.i start=[Outputs] ' \
             'id=foo caption=Outputs Block copy-button=0'
        self.assertConvert('testCaptionId.html', md)

    def testStripHeader(self):
        md = '!listing framework/src/kernels/Diffusion.C end=template copy-button=0'
        self.assertConvert('testStripHeader.html', md)

    def testDisableStripHeader(self):
        md = '!listing framework/src/kernels/Diffusion.C end=template strip-header=0 copy-button=0'
        self.assertConvert('testDisableStripHeader.html', md)

    def testLanguage(self):
        md = '!listing framework/src/kernels/Diffusion.C end=template language=foo copy-button=0'
        self.assertConvert('testLanguage.html', md)

    def testStripExtraNewLines(self):
        md = '!listing framework/src/kernels/Diffusion.C end=Diffusion::Diffusion ' \
             'strip-extra-newlines=true copy-button=0'
        self.assertConvert('testStripExtraNewLines.html', md)

    def testDisableStripExtraNewLines(self):
        md = '!listing framework/src/kernels/Diffusion.C end=Diffusion::Diffusion ' \
             'strip-extra-newlines=false copy-button=0'
        self.assertConvert('testDisableStripExtraNewLines.html', md)

    def testStripLeadingWhitespace(self):
        md = '!listing framework/src/kernels/Diffusion.C line=Kernel(parameters) ' \
             'strip-leading-whitespace=1 copy-button=0'
        self.assertConvert('testStripLeadingWhitespace.html', md)

    def testDisableStripLeadingWhitespace(self): #pylint: disable=invalid-name
        md = '!listing framework/src/kernels/Diffusion.C line=Kernel(parameters) ' \
             'strip-leading-whitespace=false copy-button=0'
        self.assertConvert('testDisableStripLeadingWhitespace.html', md)

    def testPrefixSuffix(self):
        md = '!listing framework/src/kernels/Diffusion.C line=Kernel(parameters) ' \
             'strip-leading-whitespace=1 prefix=BEFORE suffix=AFTER copy-button=0'
        self.assertConvert('testPrefixSuffix.html', md)

    def testIndent(self):
        md = '!listing framework/src/kernels/Diffusion.C line=Kernel(parameters) indent=8 ' \
             'strip-leading-whitespace=1 copy-button=0'
        self.assertConvert('testIndent.html', md)

    def testInputListing(self):
        md = '!listing test/tests/kernels/simple_diffusion/simple_diffusion.i block=Kernels ' \
             'id=diffusion_block caption=Diffusion Kernel Input Syntax copy-button=0'
        self.assertConvert('testInputListing.html', md)

    def testRef(self):
        md = '!listing test/tests/kernels/simple_diffusion/simple_diffusion.i start=[Outputs] ' \
             'id=foo copy-button=0\n\n\\ref{foo}'
        self.assertConvert('testRef.html', md)

    def testFenced(self):
        md = 'First fence without listing.\n\n```python\nx+y=1;\n```\n\n'
        md += 'Second fence without listing.\n\n```python\nx+z=2;\n```\n\n'
        md += 'Third fence.\n\n!listing caption=Third id=three\n```\nz+y=3;\n```\n\n'
        self.assertConvert('testFenced.html', md)

    def testFencedNoButton(self):
        md = 'First fence.\n\n!listing copy-button=False caption=First id=one\n```\nx+y=1;\n```\n\n'
        md += 'Second fence without listing.\n\n```python\nx+z=2;\n```'
        self.assertConvert('testFencedNoButton.html', md)

    def testListingTick(self):
        md = 'This should (`!listing`) be a paragraph.'
        self.assertConvert('testListingTick.html', md)

if __name__ == '__main__':
    unittest.main(verbosity=2)
