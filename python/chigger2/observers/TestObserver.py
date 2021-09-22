#pylint: disable=missing-docstring
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html
import os
import sys
import logging
import io
import traceback
from unittest import mock
import vtk
from moosetools import mooseutils
from moosetools import chigger
from .ChiggerObserver import ChiggerObserver

class TestObserver(ChiggerObserver):
    """
    Observer to perform testing of an active render window.
    """

    @staticmethod
    def validParams():
        opt = ChiggerObserver.validParams()
        opt.add('terminate', vtype=bool, default=True, doc="Exit after rendering.")
        opt.add('duration', vtype=(int, float), default=0.5, doc="Duration to wait in seconds for event trigger.")
        opt.add('averaging', vtype=bool, default=False, doc="Control default setting for 'averaging' of image compare.")
        opt.add('shift', vtype=bool, default=False, doc="Control default setting for 'shift' of image compare.")
        opt.add('threshold', vtype=int, default=0, doc="Control default setting for 'threshold' of image compare.")
        return opt

    def __init__(self, *args, **kwargs):
        ChiggerObserver.__init__(self, *args, **kwargs)

        self._window.getVTKInteractor().CreateOneShotTimer(int(self.getParam('duration'))*1000)
        self.addObserver(vtk.vtkCommand.TimerEvent, self._onEvent)
        self._actions = list()
        self._retcode = 0

    def status(self):
        return self._retcode

    def setObjectParams(self, obj, *args, **kwargs):
        self.assertFunction(lambda: self._setObjectParams(obj, *args, **kwargs))

    def pressKey(self, key, shift=False):
        """
        Simulate a key press.

        Inputs:
            key[str]: The key symbol (e.g. "k").
            shift[bool]: Flag for holding the shift key.
        """
        self.assertFunction(lambda: self._pressKey(key, shift))

    def assertFunction(self, func, stack=None):
        self._actions.append((func, stack or traceback.extract_stack()))

    def assertImage(self, *args, **kwargs):
        """
        Use vtkImageDifference to compare two images.

        The *filename* provided is image to be created by the chigger.Window associated with this
        observer object. This image is compared against the image provided in *goldname*, which
        defaults to a file within the a "gold" directory. For example, if *filename* is
        "image.png" the the default comparison will use "gold/image.png".

        The *threshold*, *averaging*, and *shift* set the associated value on the underlying
        vtkImageDifference object performing the comparison. The *threshold* is an integer that
        defaults to zero that sets the tolerance for pixel differences. The *averaging* is a
        boolean that toggles the use of averaging, when enabled the comparison will use averaged
        data within a 3x3 region for the comparison. The *shift* is a boolean that when True enables
        pixels to shift by two pixels between images.
        """
        self.assertFunction(lambda: self._assertImage(*args, **kwargs), traceback.extract_stack())

    def assertInConsole(self, *args, **kwargs):
        """
        Assert that the given text appears
        """
        self.assertFunction(lambda: self._assertInConsole(*args, **kwargs), traceback.extract_stack())

    def assertNotInConsole(self, *args, **kwargs):
        """
        Assert that the given text appears
        """
        self.assertFunction(lambda: self._assertNotInConsole(*args, **kwargs), traceback.extract_stack())

    def assertInLog(self, *args, **kwargs):
        """
        Assert that the given text appears
        """
        self.assertFunction(lambda: self._assertInLog(*args, **kwargs), traceback.extract_stack())

    def assertNotInLog(self, *args, **kwargs):
        """
        Assert that the given text appears
        """
        self.assertFunction(lambda: self._assertNotInLog(*args, **kwargs), traceback.extract_stack())

    def _assertImage(self, filename, goldname=None, threshold=None, averaging=None, shift=None):
        """
        see assertImage
        """
        if threshold is None: threshold = self.getParam('threshold')
        if averaging is None: averaging = self.getParam('averaging')
        if shift is None: shift = self.getParam('shift')

        # Write the current window to file
        self._window.write(imagename=filename)

        # Read the gold file, and err if not found
        goldname = goldname or os.path.join(os.path.dirname(filename), 'gold', os.path.basename(filename))
        if not os.path.exists(goldname):

            msg = "GOLD FILE DOES NOT EXIST\n"
            msg += "   GOLD: {}\n".format(goldname)
            msg += "   TEST: {}\n".format(filename)
            err = 1

        else:
            gold = vtk.vtkPNGReader()
            gold.SetFileName(goldname)

            # Create image of current window
            current = vtk.vtkWindowToImageFilter()
            current.SetInput(self._window.getVTKWindow())

            # Compute the diff
            diff = vtk.vtkImageDifference()
            diff.SetInputConnection(current.GetOutputPort())
            diff.SetImageConnection(gold.GetOutputPort())
            diff.SetThreshold(threshold)
            diff.SetAveraging(averaging)
            diff.SetAllowShift(shift)
            diff.SetAverageThresholdFactor(1)
            diff.Update()

            # Get/report error
            err = diff.GetThresholdedError()# > diff.GetThreshold()
            msg = None
            if err:

                # Write the image difference to file
                diffname = os.path.join(os.path.dirname(filename), 'diff_{}'.format(os.path.basename(filename)))
                writer = vtk.vtkPNGWriter()
                writer.SetInputConnection(diff.GetOutputPort())
                writer.SetFileName(diffname)
                writer.Write()

                # Create an error message
                msg =  "IMAGES DIFFER:\n"
                msg += "   GOLD: {}\n".format(goldname)
                msg += "   TEST: {}\n".format(filename)
                msg += "   DIFF: {}\n".format(diffname)
                msg += "  ERROR: {}\n\n".format(err)
                msg += str(diff)

        return err > 0, msg

    def _assertInLog(self, text, obj, key=None, shift=False, args=None, kwargs=None):
        formatter = chigger.ChiggerFormatter()
        func = formatter.format
        if kwargs is None: kwargs = dict()
        if args is None: args = set()
        with mock.patch('moosetools.chigger.ChiggerFormatter.format') as log:
            log.side_effect = func
            if obj is not None:
                self._setObjectParams(obj, *args, **kwargs)
            if key is not None:
                self._pressKey(key, shift=shift)

        err = False
        msg = None
        for call in log.call_args_list:
            record = call.args[0]
            err = text not in record.msg

        if err:
            msg = "TEXT IN LOG(S):\n{}".format(text)
        return err > 0, msg

    def _assertNotInLog(self, text, obj, key=None, shift=False, **kwargs):
        formatter = chigger.ChiggerFormatter()
        func = formatter.format
        with mock.patch('moosetools.chigger.ChiggerFormatter.format') as log:
            #log.side_effect = func
            if obj is not None:
                self._setObjectParams(obj, **kwargs)
            if key is not None:
                self._pressKey(key, shift=shift)

        err = False
        msg = None
        for call in log.call_args_list:
            record = call.args[0]
            err = text in record.msg

        if err:
            msg = "TEXT IN LOG(S):\n{}".format(text)

        return err > 0, msg

    def _assertInConsole(self, text, obj=None, key=None, shift=False, **kwargs):
        with mock.patch("sys.stdout", new=io.StringIO()) as out:
            if obj is not None:
                self._setObjectParams(obj, **kwargs)
            if key is not None:
                self._pressKey(key, shift=shift)

        stdout = out.getvalue()
        err = text not in stdout
        msg = None
        if err:
            msg = "TEXT NOT IN CONSOLE:\n"
            msg += "  TEXT: {}\n".format(text)
            msg += "  CONSOLE:\n{}".format(stdout)

        return err > 0, msg

    def _assertNotInConsole(self, text, obj=None, key=None, shift=False):
        with mock.patch("sys.stdout", new=io.StringIO()) as out:
            if obj is not None:
                self._setObjectParams(obj, **kwargs)
            if key is not None:
                self._pressKey(key, shift=shift)

        stdout = out.getvalue()
        err = text in stdout
        msg = None
        if err:
            msg = "TEXT IN CONSOLE:\n"
            msg += "  TEXT: {}\n".format(text)
            msg += "  CONSOLE:\n{}".format(stdout)

        return err > 0, msg

    def _pressKey(self, key, shift=False):
        vtkinteractor = self._window.getVTKInteractor()
        vtkinteractor.SetKeySym(key)
        vtkinteractor.SetShiftKey(shift)
        vtkinteractor.InvokeEvent(vtk.vtkCommand.KeyPressEvent, vtkinteractor)
        vtkinteractor.SetKeySym(None)
        vtkinteractor.SetShiftKey(False)
        return 0, None

    def _setObjectParams(self, obj, *args, **kwargs):
        obj.setParams(*args, **kwargs)
        self._window.render()
        self._window.resetClippingRange()
        self._window.resetCamera()

    def _onEvent(self, *args, **kwargs):
        self.debug("Execute event")

        for action, stack in self._actions:
            out = action()
            if out is not None:
                self._retcode += out[0]
                if out[0]:
                    msg = '{}:{}\n{}'.format(stack[-2].filename, stack[-2].lineno, out[1])
                    self.error(msg)
                    self.setParam('terminate', True)
                    break

        if self.getParam('terminate'):
            self.terminate()
