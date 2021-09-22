#!/usr/bin/env python
from unittest import mock
from moosetools import chigger

class TestWindow(chigger.base.ChiggerTestCase):
    def setUp(self):
        super().setUp()
        self._cube = chigger.geometric.Cube()

    def testDefault(self):
        self.assertImage('window_default.png')

    def testSize(self):
        self.setObjectParams(self._window, size=(100,425))
        self.assertImage('window_size.png')

    def testOffscreen(self):
        pass

        # Mock doesn't work on VTK object
        """
        self.disableWindowStart()
        self._window.setParams(offscreen=True)
        with mock.patch('vtk.vtkRenderWindow.OffScreenRenderingOn') as mock_offscreen:
            self._window.start()

        mock_offscreen.assert_called_once()
        """

#window = chigger.Window(size=(1000,300), background=(0,0.2,1))
#obs = chigger.observers.MainWindowObserver(window)
#view = chigger.Viewport(window)
#cube = chigger.geometric.Cube(view)
#window.write('window.png')
#window.start()

if __name__ == '__main__':
    import unittest
    unittest.main(verbosity=2)
