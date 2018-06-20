#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from PyQt5 import QtCore
import mooseutils
from peacock.utils import WidgetUtils

class MooseWidget(object):
    """
    A base class for extending functionality of QtWidgets.QWidget objects.
    """

    def __init__(self):
        super(MooseWidget, self).__init__()

        # Saved GUI state
        self._state = dict()

        # List of signals (populated by setup)
        self._signals = dict()

    def setup(self):
        """
        Inspects the class members.
            (1) Calls _setup methods of QObject member variables
            (2) Assigns Qt object name base on the member variable name
            (3) Stores all signals that may be accessed via signals() method.
        """

        # Clear the list of signals
        self._signals = dict()

        parent_name = self.objectName()
        if parent_name:
            parent_name += '/'

        for member in dir(self):
            if not hasattr(self, member):
                continue
            attr = getattr(self, member)
            setup = '_setup' + member
            slot = '_on' + member[0].upper() + member[1:]

            if isinstance(attr, QtCore.QObject):
                name = str(parent_name) + member
                attr.setObjectName(name)

                if hasattr(self, setup):
                    mooseutils.mooseDebug(name + "::" + setup, color='GREEN')
                    setupMethod = getattr(self, setup)
                    setupMethod(attr)

            elif isinstance(attr, QtCore.pyqtBoundSignal):
                self._signals[member] = attr
                if hasattr(self, slot):
                    mooseutils.mooseDebug(member, '-->', slot, color='MAGENTA')
                    attr.connect(getattr(self, slot))

    def dumpQObjectTree(self):
        """
        Dump the Qt object tree to the screen, this is available with Qt but only with a debug build.
        """
        WidgetUtils.dumpQObjectTree(self)

    def stateKey(self):
        """
        Return a unique "key" for saving widget state, see ExodusPlugin.
        """
        return 'default'

    def hasState(self, *args, **kwargs):
        key = kwargs.pop('key', self.stateKey())
        if not args:
            args = [self]

        all_state = []
        for widget in args:
            state = widget.property('state')
            all_state.append((key in state) if state else False)

        return all(all_state)

    def store(self, *args, **kwargs):
        """
        Store the widget state.

        Args:
            *args[list]: List of widgets to store, if not provided self is used.

        Kwargs:
            passed to peacock.utils.WidgetUtils.storeWidget
        """
        self.blockSignals(True)
        key = kwargs.pop('key', self.stateKey())
        if args:
            for widget in args:
                WidgetUtils.storeWidget(widget, key, **kwargs)
        else:
            WidgetUtils.storeWidget(self, key, **kwargs)
        self.blockSignals(False)

    def load(self, *args, **kwargs):
        """
        Load the state of the widget.

        Args:
            key[str]: The key to which the current settings should be stored.
            *args[list]: List of widgets to store, if not provided self is used.

        Kwargs:
            passed to peacock.utils.WidgetUtils.storeWidget
        """
        key = kwargs.pop('key', self.stateKey())
        if args:
            for widget in args:
                WidgetUtils.loadWidget(widget, key, **kwargs)
        else:
            WidgetUtils.loadWidget(self, key, **kwargs)

    def signals(self):
        """
        Return the list of available signals.
        """
        return self._signals
