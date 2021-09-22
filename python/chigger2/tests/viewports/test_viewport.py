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
import chigger

def main():

    window = chigger.Window(size=(400, 400))

    left = chigger.Viewport(xmax=0.5)
    c_left = chigger.geometric.Cube()
    right = chigger.Viewport(xmin=0.5)
    c_right = chigger.geometric.Cube()


    test = chigger.observers.TestObserver(terminate=False)
    test.assertImage('initial.png')

    # location
    test.setObjectParams(left, xmin=0.1, xmax=0.4)
    test.assertImage('location.png')

    test.setObjectParams(left, xmin=0, xmax=0.5)
    test.assertImage('initial.png')

    # xmin/ymin/xmax/ymax range check
    test.assertInLog("Value must be in range [0,1]", left, xmin=1980)
    test.assertInLog("Value must be in range [0,1]", left, xmax=1980)
    test.assertInLog("Value must be in range [0,1]", left, ymin=1980)
    test.assertInLog("Value must be in range [0,1]", left, ymax=1980)

    test.setObjectParams(left, highlight=True)
    test.assertImage('highlight.png')

    test.pressKey('s')
    test.assertImage('source_initial.png')


    window.start()


class TestHelp(unittest.TestCase):
    def test(self):
        self.assertFalse(main(), "Failed to execute script without errors.")

if __name__ == '__main__':
    import sys
    sys.exit(main())
