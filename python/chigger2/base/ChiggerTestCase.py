import sys
import unittest
from moosetools import chigger

@unittest.skipIf(sys.version_info.major == 3 and sys.version_info.minor < 8, "Requires python 3.8 or greater")
class ChiggerTestCase(unittest.TestCase):
    """
    Base test class for testing chigger functionality.

    This class acts as a wrapper of the chigger.observer.TestObserver object. By default it setups
    a new chigger.Window and chigger.Viewport with the necessary observer for each test.
    """
    SIZE = (300, 300)

    def setUp(self):
        self._window = chigger.Window(size=self.SIZE)
        self._viewport = chigger.Viewport()
        self._test = chigger.observers.TestObserver()
        self._start = True

    def pressKey(self, *args, **kwargs):
        self._test.pressKey(*args, **kwargs)

    def disableWindowStart(self):
        self._start = False

    def setObjectParams(self, obj, *args, **kwargs):
        self._test.setObjectParams(obj, *args, **kwargs)

    def assertImage(self, *args, **kwargs):
        self._test.assertImage(*args, **kwargs)

    def assertInConsole(self, *args, **kwargs):
        self._test.assertInConsole(*args, **kwargs)

    def assertNotInConsole(self, *args, **kwargs):
        self._test.assertInConsole(*args, **kwargs)

    def assertInLog(self, *args, **kwargs):
        self._test.assertInLog(*args, **kwargs)

    def assertNotInLog(self, *args, **kwargs):
        self._test.assertNotInLog(*args, **kwargs)

    def _callTestMethod(self, method):
        """Only exists in python 3.8 or greater."""
        unittest.TestCase._callTestMethod(self, method)
        if self._start:
            self._window.start()
            self.assertFalse(self._test.status())
        self._start = True
