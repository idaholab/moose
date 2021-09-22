#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html
from moosetools import chigger

class TestFont(chigger.base.ChiggerTestCase):
    def setUp(self):
        super().setUp()
        self._text = chigger.annotations.Text(text='This is a test.')

    def testDefault(self):
        self.assertImage('default.png')

    def testColor(self):
        self.setObjectParams(self._text, font_color=chigger.utils.Color(1,0,1))
        self.assertImage('font_color.png')

    def testOpacity(self):
        self.setObjectParams(self._text, font_opacity=0.2)
        self.assertImage('font_opacity.png')

    def testSize(self):
        self.setObjectParams(self._text, font_size=1)
        self.assertImage('font_size.png')

    def testItalic(self):
        self.setObjectParams(self._text, font_italic=True)
        self.assertImage('font_italic.png')

    def testBold(self):
        self.setObjectParams(self._text, font_bold=True)
        self.assertImage('font_bold.png')

    def testFamily(self):
        self.setObjectParams(self._text, font_family='times')
        self.assertImage('font_family.png')

    def testErrors(self):
        self.assertInLog("The supplied value must be in range [0,1]",
                         self._text, kwargs=dict(font_opacity=1980))

        self.assertInLog("The supplied size must be in range (0,1]",
                         self._text, kwargs=dict(font_size=1980))

        self.assertInLog("Attempting to set 'family' to a value of",
                         self._text, kwargs=dict(font_family='wrong'))


class TestFrame(chigger.base.ChiggerTestCase):
    def setUp(self):
        super().setUp()
        self._text = chigger.annotations.Text(text='CHIGGER', position=(0.5, 0.5), halign='center')

    def testOn(self):
        self.setObjectParams(self._text, 'frame', on=True)
        self.assertImage('frame_on.png')

    def testColor(self):
        self.setObjectParams(self._text, 'frame', on=True, width=10, color=(0.2, 0.7, 0.8))
        self.assertImage('frame_color.png')

    def testWidth(self):
        self.setObjectParams(self._text, frame_on=True, frame_width=10)
        self.assertImage('frame_width.png')

class TestBackground(chigger.base.ChiggerTestCase):
    def setUp(self):
        super().setUp()
        self._text = chigger.annotations.Text(text='CHIGGER', position=(0.5, 0.5), halign='center')

    def testColor(self):
        self.setObjectParams(self._text, background_color=chigger.utils.Color(1,0,1))
        self.assertImage('background_color.png')

    def testOpacity(self):
        self.setObjectParams(self._text, 'background', color=chigger.utils.Color(1,0,1), opacity=0.2)
        self.assertImage('background_opacity.png')

    def testErrors(self):
        self.assertInLog("The supplied value must be in range (0,1]",
                         self._text, kwargs=dict(background_opacity=1980))


class TestRotate(chigger.base.ChiggerTestCase):
    def setUp(self):
        super().setUp()
        self._text = chigger.annotations.Text(text='CHIGGER', position=(0.5, 0.5), halign='center')

    def testRotate45(self):
        self.setObjectParams(self._text, rotate=45)
        self.assertImage('rotate45.png')

    def testRotate90(self):
        self.setObjectParams(self._text, rotate=90)
        self.assertImage('rotate90.png')

    def testRotate135(self):
        self.setObjectParams(self._text, rotate=135)
        self.assertImage('rotate135.png')

    def testRotate225(self):
        self.setObjectParams(self._text, rotate=225)
        self.assertImage('rotate225.png')

    def testRotate270(self):
        self.setObjectParams(self._text, rotate=270)
        self.assertImage('rotate270.png')

    def testRotate315(self):
        self.setObjectParams(self._text, rotate=315)
        self.assertImage('rotate315.png')

    def testErrors(self):
        self.assertInLog("The supplied value must in range [0,360)",
                         self._text, kwargs=dict(rotate=1980))


class TestHAlign(chigger.base.ChiggerTestCase):
    def setUp(self):
        super().setUp()
        self._text = chigger.annotations.Text(text='CHIGGER', position=(0.5, 0.5))

    def testLeft(self):
        self.setObjectParams(self._text, halign='left')
        self.assertImage('halign_left.png')

    def testCenter(self):
        self.setObjectParams(self._text, halign='center')
        self.assertImage('halign_center.png')

    def testRight(self):
        self.setObjectParams(self._text, halign='right')
        self.assertImage('halign_right.png')

class TestVAlign(chigger.base.ChiggerTestCase):
    def setUp(self):
        super().setUp()
        self._text = chigger.annotations.Text(text='A', font_size=0.5, position=(0.5, 0.5), halign='center')

    def testBottom(self):
        self.setObjectParams(self._text, valign='bottom')
        self.assertImage('valign_bottom.png')

    def testCenter(self):
        self.setObjectParams(self._text, valign='center')
        self.assertImage('valign_center.png')

    def testTop(self):
        self.setObjectParams(self._text, valign='top')
        self.assertImage('valign_top.png')

class TestAutoColorText(chigger.base.ChiggerTestCase):
    def setUp(self):
        super().setUp()
        self._text = chigger.annotations.Text(text='chigger', font_size=0.2, position=(0.5, 0.5), halign='center')

    def testDark(self):
        self._window.setParams(background_color=chigger.utils.Color(0.8, 0.8, 0.8))
        self.assertImage('auto_color_dark.png')

    def testLight(self):
        self._window.setParams(background_color=chigger.utils.Color(0.2, 0.2, 0.2))
        self.assertImage('auto_color_light.png')

class TestMath(chigger.base.ChiggerTestCase):
    def setUp(self):
        super().setUp()
        self._text = chigger.annotations.Text(text='$\\rho C_p\\frac{\\partial T}{\\partial t} - \\nabla\\cdot(k\\nabla T) = s$',
                                              font_size=0.1, position=(0.5, 0.5), halign='center')

    def test(self):
        self.assertImage('math.png')


if __name__ == '__main__':
    import unittest
    unittest.main(verbosity=2, buffer=False)
