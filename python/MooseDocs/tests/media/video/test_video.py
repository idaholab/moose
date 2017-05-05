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

class TestVideo(MarkdownTestCase):
    EXTENSIONS = ['MooseDocs.extensions.media']

    def testDefault(self):
        md = '!media http://clips.vorwaerts-gmbh.de/VfE.webm'
        self.assertConvert('testDefault.html', md)

    def testSettings(self):
        md = '!media http://clips.vorwaerts-gmbh.de/VfE.webm video-width=100% autoplay=True'
        self.assertConvert('testSettings.html', md)

    def testDefaultId(self):
        md = '!media http://clips.vorwaerts-gmbh.de/VfE.webm id=foo caption=A video'
        self.assertConvert('testDefaultId.html', md)

if __name__ == '__main__':
    unittest.main(verbosity=2)
