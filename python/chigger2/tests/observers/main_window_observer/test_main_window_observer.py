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
from moosetools import chigger

def main():

    window = chigger.Window(size=(400, 400))

    left = chigger.Viewport(xmax=0.5)
    rect0 = chigger.geometric.Rectangle(xmin=0.25, xmax=0.5, ymin=0.25, ymax=0.75, color=chigger.utils.Color(0.5, 0.1, 0.2))
    cube0 = chigger.geometric.Cube(xmin=0.5, xmax=0.8, ymin=0, ymax=0.5, zmin=0.8, zmax=1, color=chigger.utils.Color(0.1, 0.2, 0.8))

    right = chigger.Viewport(xmin=0.5)
    rect1 = chigger.geometric.Rectangle(xmin=0.25, xmax=0.5, ymin=0.25, ymax=0.75, color=chigger.utils.Color(0.2, 0.1, 0.5))
    cube1 = chigger.geometric.Cube(xmin=0.5, xmax=0.8, ymin=0, ymax=0.5, zmin=0.8, zmax=1, color=chigger.utils.Color(0.8, 0.2, 0.1))

    test = chigger.observers.TestObserver(terminate=False)
    test.assertImage('initial.png')

    # General keybindings
    test.assertInConsole('MainWindowObserver Keybindings', key='h')
    test.assertInConsole('Display the help for this object.', key='h')

    # Move forward through source
    test.pressKey('s')
    test.assertImage('active_source1.png')

    test.pressKey('s')
    test.assertImage('active_source2.png')

    test.pressKey('s')
    test.assertImage('active_source3.png')

    test.pressKey('s')
    test.assertImage('active_source4.png')

    test.pressKey('s')
    test.assertImage('initial.png')

    # Move backward through source
    test.pressKey('s', shift=True)
    test.assertImage('active_source4.png')

    test.pressKey('s', shift=True)
    test.assertImage('active_source3.png')

    test.pressKey('s', shift=True)
    test.assertImage('active_source2.png')

    test.pressKey('s', shift=True)
    test.assertImage('active_source1.png')

    test.pressKey('s', shift=True)
    test.assertImage('initial.png')

    # Select source and clear selection
    test.pressKey('s', shift=True)
    test.assertImage('active_source4.png')

    test.pressKey('t')
    test.assertImage('initial.png')

    # Move forward through viewports
    test.pressKey('v')
    test.assertImage('active_viewport1.png')

    test.pressKey('v')
    test.assertImage('active_viewport2.png')

    test.pressKey('v')
    test.assertImage('initial.png')

    # Move backward through viewports
    test.pressKey('v', shift=True)
    test.assertImage('active_viewport2.png')

    test.pressKey('v', shift=True)
    test.assertImage('active_viewport1.png')

    test.pressKey('v', shift=True)
    test.assertImage('initial.png')

    # Select viewport and clear selection
    test.pressKey('v', shift=True)
    test.assertImage('active_viewport2.png')

    test.pressKey('t')
    test.assertImage('initial.png')

    # Select source, then viewport, then source
    test.pressKey('s')
    test.assertImage('active_source1.png')

    test.pressKey('v')
    test.assertImage('active_viewport1.png')

    test.pressKey('s')
    test.assertImage('active_source1.png')

    # Printing of options
    test.assertInConsole('Rectangle Available Params:', key='p')
    test.assertInConsole('Rectangle -> setParams', key='p', shift=True)

    # Printing of camera
    test.assertInConsole('camera = vtk.vtkCamera()', key='c')

    test.pressKey('q')
    window.start()

    return test.status()

class TestHelp(unittest.TestCase):
    def test(self):
        self.assertFalse(main(), "Failed to execute script without errors.")

if __name__ == '__main__':
    import sys
    sys.exit(main())
