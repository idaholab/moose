#pylint: disable=missing-docstring
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html
import textwrap
import vtk

import mooseutils
from ChiggerObserver import ChiggerObserver
from .. import utils
from ..geometric import OutlineResult

class MainWindowObserver(ChiggerObserver, utils.KeyBindingMixin):
    """
    The main means for interaction with the chigger interactive window.
    """

    @staticmethod
    def validOptions():
        opt = ChiggerObserver.validOptions()
        return opt

    @staticmethod
    def validKeyBindings():
        bindings = utils.KeyBindingMixin.validKeyBindings()
        bindings.add('r', MainWindowObserver._nextResult, desc="Select the next result object.")
        bindings.add('r', MainWindowObserver._previousResult, shift=True, desc="Select the previous result object.")
        bindings.add('t', MainWindowObserver._deactivateResult, desc="Clear selection(s).")
        bindings.add('h', MainWindowObserver._printHelp, desc="Display the help for this object.")
        return bindings

    def __init__(self, **kwargs):
        super(MainWindowObserver, self).__init__(**kwargs)

        self.__outline_result = None

    def init(self, *args, **kwargs):
        """
        Add the KeyPressEvent and MouseMoveEvent for this object.
        """
        super(MainWindowObserver, self).init(*args, **kwargs)

        self._window.getVTKInteractor().AddObserver(vtk.vtkCommand.KeyPressEvent,  self._onKeyPressEvent)
        self._window.getVTKInteractor().AddObserver(vtk.vtkCommand.MouseMoveEvent, self._onMouseMoveEvent)

    def _nextResult(self, window, binding):
        window.nextActive()
        active = window.getActive()

    def _previousResult(self, window, binding):
        window.nextActive(reverse=True)

    def _deactivateResult(self, window, binding):
        window.setActive(None)

    def _printHelp(self, window, binding):
        """
        Display the available controls for this object.
        """

        # Object name/type
        print mooseutils.colorText('General Keybindings:', 'YELLOW')
        self.__printKeyBindings(self.keyBindings())

        active = window.getActive()
        if active is not None:
            print mooseutils.colorText('\n{} Keybindings:'.format(active.title()), 'YELLOW')
            self.__printKeyBindings(active.keyBindings())

    @staticmethod
    def __printKeyBindings(bindings):
        n = 0
        out = []
        for key, value in bindings.iteritems():
            tag = 'shift-{}'.format(key[0]) if key[1] else key[0]
            desc = [item.description for item in value]
            out.append([tag, '\n\n'.join(desc)])
            n = max(n, len(tag))

        for key, desc in out:
            key = mooseutils.colorText('{0: >{w}}: '.format(key, w=n), 'GREEN')
            print '\n'.join(textwrap.wrap(desc, 100, initial_indent=key, subsequent_indent=' '*(n + 2)))

    def _onKeyPressEvent(self, obj, event): #pylint: disable=unused-argument
        """
        The function to be called by the RenderWindow.

        Inputs:
            obj, event: Required by VTK.
        """
        key = obj.GetKeySym().lower()
        shift = obj.GetShiftKey()

        #for binding in self._window.getActive().getKeyBindings(key, shift):
        for binding in self.getKeyBindings(key, shift):
            binding.function(self, self._window, binding)

        active = self._window.getActive()
        if active:
            for binding in active.getKeyBindings(key, shift):
                binding.function(active, self._window, binding)

        self._window.update()

    def _onMouseMoveEvent(self, obj, event):
        result = self._window.getActive()
        if (result is not None) and hasattr(result, 'onMouseMoveEvent'):
            loc = obj.GetEventPosition()
            sz = result.getVTKRenderer().GetSize()
            position=(loc[0]/float(sz[0]), loc[1]/float(sz[1]))
            result.onMouseMoveEvent(position)
            self._window.update()
