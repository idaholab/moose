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
