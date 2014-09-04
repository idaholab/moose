import sys

#from PySide import QtCore, QtGui

#import inspect
#from src.utils import *
#from src.base import MooseWidget


##
# A MooseWidget base class for information handling.
#
# This class provides an info() method for printing information
# about the class objects, callbacks, pulls, and signals
class MooseWidgetInfoBase(object):
  def __init__(self):
    pass
    #if not isinstance(self, MooseWidget) or not isinstance(self, QtGui.QWidget):
    #  print 'MooseWidgetInfoBase only works if the parent class is both a MooseWidget and a QtGui.QWidget'
    #  sys.exit()
  ##
  # Displays the object and signals for this object (public)
  #
  def info(self):

    # Build the object data to print
    data = []
    self._getObjectInfo(data)
    print '\n', self.__class__.__name__, 'Objects:'
    self.__printTable(['Handle', 'Parent', 'Type', 'Setup', 'Callback'], data)

    # Extract the signal information
    signals = []
    self._getSignalInfo(signals)
    print '\n', self.__class__.__name__, 'Signals:'
    self.__printTable(['Signal', 'Parent'], signals)

    # Extract the pull information
    pulls = []
    self._getPullInfo(pulls)
    print '\n', self.__class__.__name__, 'Pulls:'
    self.__printTable(['Pull', 'Parent'], pulls)
    print '\n'

  ##
  # Recursively, gather object information for this MooseObject
  # @param data The data list to populate
  # @param indent_level The level of indentation to applied to the table entry
  #
  def _getObjectInfo(self, data, indent_level=0):
    for key, obj in self._objects.iteritems():
      has_setup = hasattr(self, '_setup' + key)
      has_callback = hasattr(self, '_callback' + key)
      flavor = obj.__class__.__name__
      has_setup = hasattr(self, '_setup' + key)
      has_callback = hasattr(self, '_callback' + key)
      flavor = obj.__class__.__name__
      parent = obj.property('parent_name')
      data.append(['  '*indent_level + key, parent, flavor, has_setup, has_callback])
      if isinstance(obj, MooseWidget):
        obj._getObjectInfo(data, indent_level + 1)
    return data

  ##
  # Recursively, gather signal information for this MooseObject
  # @param data The data list to populate
  def _getSignalInfo(self, data):

    # Search for signals in this object
    for item in dir(self):
      if item.startswith('_signal_') and isinstance(getattr(self, item), QtCore.Signal):
        data.append([item, self.__class__.__name__])

    # Search for signals in child objects
    for key, obj in self._objects.iteritems():
      if isinstance(obj, MooseWidget):
        obj._getSignalInfo(data)

  ##
  # Recursively, gather pull information for this MooseObject
  # @param data The data list to populate
  def _getPullInfo(self, data):

    # Search for signals in this object
    for item in dir(self):
      if item.startswith('_pull'):
        data.append([item, self.__class__.__name__])

    # Search for signals in child objects
    for key, obj in self._objects.iteritems():
      if isinstance(obj, MooseWidget):
        obj._getPullInfo(data)


# private:
  ##
  # Given header and data, create a formatted table
  def __printTable(self, header, data):

    # Print empty message if no data exists
    if len(data) == 0:
      print ' No data'
      return

    # Insert the header to the beginning of the data
    data.insert(0, header)

    # Compute the table widths
    widths = [0]*len(data[0])
    for d in data:
      for i in xrange(len(d)):

        # Convert bool
        if isinstance(d[i], bool):
          if d[i]:
            d[i] = 'True'
          else:
            d[i] = 'False'

        # Compute max width
        widths[i] = max(widths[i], len(str(d[i])))

    # Strip header from the beginning
    data.pop(0)

    # Create format strings for building signal table
    cnt = 0
    frmt = ''
    line = ''
    for w in widths:
      frmt += ' {' + str(cnt) + ':' + str(w) + 's}'
      line += ' ' + '-'*(w)
      cnt += 1

    # Print signals
    print line
    print frmt.format(*header)
    print line
    for d in data:
      print frmt.format(*d)
    print line
