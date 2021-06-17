#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

"""
Sorted container for storing objects.
"""

class Storage(object):
    """
    Container for storing objects by name with the ability to insert relative to existing objects.

    Inputs:
      s_type: The type of object to store, this is provided for error checking.
    """
    def __init__(self, stype=None):

        # The type of Element allowed to be stored
        self._type = stype

        # The names of the objects as assigned when added
        self._keys = []

        # The objects being stored, a list is used for fast looping, consequently
        self._objects = []

    def add(self, key, obj, location='_end'):
        """
        Adds the 'obj' class with the given name to the storage container.

        Args:
            key[str]: The name of the object being added
            obj[stype]: The object to store, where stype is defined in constructor (see constructor)
            location[int]: The integer location to insert the item
            location[str]: The name of the key where this should be inserted, it is possible to pass
                           special locations:
                               '_end': Insert the key being added to the end of the container
                               '_begin': Insert the key being added to the beginning of container
                               '<key': Insert the new key before the key given with the '<' prefix
                               '>key': Insert the new key after the key given with the '<' prefix
                               '=key': Replace existing key with the the new key
        """

        # Check the type
        if (self._type is not None) and (not isinstance(obj, self._type)):
            msg = 'Incorrect object provided, expected {} but received {}'
            msg = msg.format(self._type.__name__, type(obj).__name__)
            raise TypeError(msg)

        # Check if key exists
        if key in self._keys:
            raise ValueError("The key '{}' already exists.".format(key))

        # Determine the index
        insert = True
        index = None
        if isinstance(location, str):
            if location == '_end':
                index = len(self._keys)
            elif location == '_begin':
                index = 0
            elif location.startswith('<'):
                index = self._keys.index(location[1:])
            elif location.startswith('>'):
                index = self._keys.index(location[1:]) + 1
            elif location.startswith('='):
                index = self._keys.index(location[1:])
                insert = False
            else:
                index = self._keys.index(location)
        elif isinstance(location, int):
            index = location
        else:
            raise TypeError("The supplied input must be of the 'str' or 'int'.")

        if insert:
            self._keys.insert(index, key)
            self._objects.insert(index, obj)
        else:
            self._keys[index] = key
            self._objects[index] = obj

    def __getitem__(self, key):
        """
        Return class type by key.
        """
        if isinstance(key, int):
            index = key
        elif isinstance(key, str):
            index = self._keys.index(key)
        else:
            raise TypeError("The supplied type must be 'int' or 'str' but {} given." \
                            .format(type(key).__name__))

        return self._objects[index]

    def __contains__(self, key):
        """
        Check if key, index is valid.
        """
        if isinstance(key, int):
            return key < len(self._keys)
        return key in self._keys

    def __iter__(self):
        """
        Enables iteration over the Element classes stored in this container.
        """
        for obj in self._objects:
            yield obj

    def __len__(self):
        """
        Return the number of items stored.
        """
        return len(self._keys)

    def items(self):
        """
        Return key, value iterator items.
        """
        for key, obj in zip(self._keys, self._objects):
            yield key, obj
