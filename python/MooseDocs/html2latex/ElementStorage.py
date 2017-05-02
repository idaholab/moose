#pylint: disable=missing-docstring
####################################################################################################
#                                    DO NOT MODIFY THIS HEADER                                     #
#                   MOOSE - Multiphysics Object Oriented Simulation Environment                    #
#                                                                                                  #
#                              (c) 2010 Battelle Energy Alliance, LLC                              #
#                                       ALL RIGHTS RESERVED                                        #
#                                                                                                  #
#                            Prepared by Battelle Energy Alliance, LLC                             #
#                               Under Contract No. DE-AC07-05ID14517                               #
#                               With the U. S. Department of Energy                                #
#                                                                                                  #
#                               See COPYRIGHT for full restrictions                                #
####################################################################################################
#pylint: enable=missing-docstring
import mooseutils
import elements

class ElementStorage(object):
    """
    Container for storing element.Element objects. This is used with the Extension object
    and allows Element objects to be added to the conversion.

    Args:
      etype: The type of elements.Element object to store (i.e., InlineElement or BlockElement)
    """
    def __init__(self, etype=elements.Element):

        #: The type of Element allowed to be stored
        self._type = etype

        # The names of the objects as assigned when added
        self._keys = []

        # The classes to be instantiated to convert html to latex
        self._objects = []

    def add(self, key, obj, location='_end'):
        """
        Adds the 'obj' class with the given name to the storage container.

        Args:
          key[str]: The name of the object being added
          obj[Element]: The class (not isinstance) of the Element object to store
          location[str]: The name of the key where this should be inserted, it is possible to pass
                  special locations:
                    '_end': Insert the key being added to the end of the container
                    '_begin': Insert the key being added to the beginning of the container
                    '<key': Insert the new key before the key given with the '<' prefix
                    '>key': Insert the new key after the key given with the '<' prefix
                    'key': Insert the new key after the key (same as '<' prefix)
        """

        if not isinstance(obj, self._type):
            msg = 'Incorrect object provide, expected %s but received %s'
            raise mooseutils.MooseException(msg, self._type, obj)

        index = None
        if location == '_end':
            index = len(self._keys)
        elif location == '_begin':
            index = 0
        elif location.startswith('<'):
            index = self._keys.index(location[1:])
        elif location.startswith('>'):
            index = self._keys.index(location[1:]) + 1
        else:
            index = self._keys.index(location)

        self._keys.insert(index, key)
        self._objects.insert(index, obj)

    def __iter__(self):
        """
        Enables iteration over the Element classes stored in this container.
        """
        for obj in self._objects:
            yield obj
