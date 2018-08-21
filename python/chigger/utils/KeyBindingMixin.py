#pylint: disable=missing-docstring
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html
import collections
import copy

KeyBinding = collections.namedtuple('KeyBinding', 'key shift description function')

class KeyBindings(object):
    """
    A container for storing keybindings.

    This container is needed to allow for a staticmethod at the class level for populating
    the available bindings. The static method is needed to allow for MooseDocs to extract
    the bindings without instantiating the object.
    """
    def __init__(self):
        self.bindings = collections.OrderedDict()

    def add(self, key, func, shift=False, desc=None):
        """
        Add a keybinding.
        """
        if (key, shift) not in self.bindings:
            self.bindings[(key, shift)] = set()
        self.bindings[(key, shift)].add(KeyBinding(key, shift, desc, func))

class KeyBindingMixin(object):
    """
    Class for inheriting key binding support for an object.
    """

    @staticmethod
    def validKeyBindings():
        return KeyBindings()

    def __init__(self):
        super(KeyBindingMixin, self).__init__()
        self.__keybindings = self.validKeyBindings()

    def keyBindings(self):
        return copy.copy(self.__keybindings.bindings)

    def getKeyBindings(self, key, shift=False):
        return self.__keybindings.bindings.get((key, shift), set())
