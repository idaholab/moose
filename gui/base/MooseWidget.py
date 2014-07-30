from PySide import QtCore, QtGui

import inspect
from utils import *
from base import *

##
# Basic QWidget to serve as a container for controls
class MooseWidget(QtGui.QWidget):

  ##
  # Constructor.
  # @param kwargs A list of property value pairs (see below)
  #
  # Optional key=value Pairs:
  #  alignment 'horizontal' | {'vertical'}
  #    Controls the orientation for the underlying layout for this QWidget object
  #
  #  debug True | {False}
  #    Toggles the printing of debug messages, to create a debug message
  #    call the self._debug('Your message here') method. The printing of
  #    debug messages will be set on child MooseWidget object to that of the parent
  #
  #  main <QMainWindow>
  #    Sets the main window for this object, which is used for adding
  #    QMenu items. The main window object of  will be set on child MooseWidget object
  #    to that of the parent
  def __init__(self, **kwargs):

    # Call the base class constructor
    QtGui.QWidget.__init__(self)

    # All object added via addObject are stored in a dictionary
    self._objects = dict()

    # Store the debug and QMainWindow as properties on this QWidget
    self.setProperty('debug', kwargs.pop('debug', False))
    self.setProperty('main', kwargs.pop('main', None))

    # Define the main QLayout and set it for this Widget
    align = kwargs.pop('alignment', 'vertical')
    if align == 'vertical':
      self._main_layout = QtGui.QVBoxLayout()
    elif align == 'horizontal':
      self._main_layout = QtGui.QHBoxLayout()
    else:
      peacockError('Unknown alignment parameter ' + align + ', must be \'vertical\' or \'horizontal\'')
    self.setLayout(self._main_layout)

    # Create an error message dialog
    self._error_message = QtGui.QErrorMessage(self)

  ##
  # Return an object that was added via addObject (public)
  # @param handle The handle (<str>) of the object
  # @param search_children If True (default) search all children MooseWidget objects for the handle
  def object(self, handle, search_children = True):

    # Search this object for the handle
    if handle in self._objects:
      return self._objects[handle]

    # Search the children for the handle, if desired
    child_obj = None
    if search_children:
      for key, obj in self._objects.iteritems():
        if isinstance(obj, MooseWidget):
          child_obj = obj.object(handle)
          if child_obj == None:
            continue

    # If an object has not been returned yet, produce an error
    return child_obj

  ##
  # Extract a callback method by name
  # @param callback_name The handle associated with the QObject added via addObject that
  #                      has a _callback<name> method defined in the class
  # @param search_children Toggle the searching of child MooseWidget objects (optional, default is True)
  #
  def callback(self, name, search_children = True):
    callback_name = '_callback' + name

    for key, obj in self._objects.iteritems():
      if hasattr(obj, callback_name):
        return getattr(obj, callback_name)

      elif search_children and isinstance(obj, MooseWidget):
        attr = obj.callback(name)
        if attr != None:
          return attr

    #peacockWarning(...)
    return None

  ##
  # Extract a Signal object by name
  # @param name The signal name contained within a Peacock object (_signal_<name>)
  # @param search_children Toggle the searching of child MooseWidget objects (optional, default is True)
  def signal(self, name, search_children = True):
    signal_name = '_signal_' + name

    for key, obj in self._objects.iteritems():
      if hasattr(obj, signal_name):
        return getattr(obj, signal_name)

      elif search_children and isinstance(obj, MooseWidget):
        attr = obj.signal(name)
        if attr != None:
          return attr

    #peacockWarning(...)
    return None

  ##
  # Extract value from pull method  by name
  # @param name The pull method name contained within a Peacock object (_pull<Name>)
  # @param *args Additional arguments
  # @return The actual function to call
  #
  # This does not execute the method queried, but returns you the actual function, e.g.:
  #   func = obj.pull('MyFinger')
  #   data = func(some_additional_argument)
  #
  # By default, pull will automatically search "sibling" objects if the, for example.
  #
  def pull(self, name, search_owner = True):
    pull_name = '_pull' + name

    if search_owner and isinstance(self.property('owner'), MooseWidget):
      return self.property('owner').pull(name, False)

    else:
      for key, obj in self._objects.iteritems():
        if hasattr(obj, pull_name):
          attr = getattr(obj, pull_name)
          return attr

        elif isinstance(obj, MooseWidget):
          attr = obj.pull(name, False)
          if attr != None:
            return attr

    #peacockWarning(...)
    return None

  ##
  # Return true if a handle exists (public)
  # @param handle The handle (<str>) of the object
  # @param search_children If True (default) search all children MooseWidget objects for the handle
  def hasObject(self, handle, search_children = True):

    # Search this object for the handle
    if handle in self._objects:
      return True

    # Search the children for the handle, if desired
    if search_children:
      for key, obj in self._objects.iteritems():
        if isinstance(obj, MooseWidget):
          return True

    # If an object has not been returned yet, it doesn't exist
    return False


  ##
  # Add a QObject to this MooseWidget (public)
  # @param q_object The instance of the QObject to add
  # @param kwargs A list of property value pairs (see below)
  #
  # Optional key=value Pairs:
  #  handle <str>
  #    The handle of the object being created, by default the name "object_xx" is used,
  #    where xx is one pluss the number of objects already stored by this object
  #
  #  parent <str>
  #    The handle (<str>) of the parent object to which the supplied
  #    QObject is to be added to. By default the object is added to the main_layout
  #    for this object.
  #
  #  label <str>
  #    A string containing the label to add to this object. If supplied a QLabel object
  #    is automatically created with the handle set to "<handle>Layout" using the
  #    handle supplied for the current call to addObject.
  def addObject(self, q_object, **kwargs):

    # Make sure the the object added is QtGui.QObject instance
    if not isinstance(q_object, QtCore.QObject):
      peacockError("The supplied object must be a QtGui.QObject, but a ",
                   q_object.__class__.__name__, " was supplied")

    # Pass the debug and main window properties to MooseWidget objects being added
    if isinstance(q_object, MooseWidget):
      q_object.setProperty('debug', self.property('debug'))
      q_object.setProperty('main', self.property('main'))

    # Define the parent name and object
    if 'parent' in kwargs:
      parent = kwargs.pop('parent')
      parent_object = self._objects[parent]
    else:
      parent = self.__class__.__name__ + '._main_layout'
      parent_object = self._main_layout

    # Store the parent object name as a property of the object, used by info()
    q_object.setProperty('parent_name', parent)

    # Store the this object in the object being added
    q_object.setProperty('owner', self)

    # Determine the handle for the object being added, and test that it doesn't exist
    handle = kwargs.pop('handle', 'object_' + str(len(self._objects)))
    if handle in self._objects:
      peacockError("The handle, ", handle, " already exists")

    # Add the object to the list of objects and set the handle property
    self._objects[handle] = q_object
    q_object.setProperty('handle', handle)

    # Apply the label
    label = kwargs.pop('label', None)
    if label != None:
      label_handle = handle + 'Label'

      if label_handle in self._objects:
        peacockError('The handle, ', label_handle, ' already exists')

      if parent.endswith('._main_layout'):
        label_object = self.addObject(QtGui.QLabel(label), handle=handle+'Label')
      else:
        label_object = self.addObject(QtGui.QLabel(label), handle=handle+'Label', parent=parent)

      self._objects[handle].setProperty('label', label_object)

    # Depending on the object being added and the type of the parent, the action required
    # to define the child/parent relationship differs.

    # QWidget -> QTabWidget
    if isinstance(q_object, QtGui.QWidget) and isinstance(parent_object, QtGui.QTabWidget):
      self._debug('Adding QWidget (' + handle + ') as a Tab (' + parent + ')')
      parent_object.addTab(q_object, handle)

    # QAction -> QMenu
    elif isinstance(q_object, QtGui.QAction) and isinstance(parent_object, QtGui.QMenu):
      self._debug('Adding QAction (' + handle + ') to QMenu (' + parent + ')')
      parent_object.addAction(q_object)

    # QMenu -> QMenuBar
    elif isinstance(q_object, QtGui.QMenu) and isinstance(parent_object, QtGui.QMenuBar):
      self._debug('Adding QMenu (' + handle + ') to (' + parent + ')')
      parent_object.addMenu(q_object)

    # QMenu -> QMainWindow.menuBar (if 'main' property was set in MooseWidget)
    elif isinstance(q_object, QtGui.QMenu) and self.property('main') != None:
      self._debug('Adding QMenu (' + handle + ') to (QMainWindow.menuBar())')
      print self.property('main')
      menu_bar = self.property('main').menuBar()
      menu_bar.addMenu(q_object)

    # QWidget -> QLayout
    elif isinstance(q_object, QtGui.QWidget) and isinstance(parent_object, QtGui.QLayout):
      self._debug('Adding QWidget (' + handle + ') to QLayout (' + parent + ')')
      parent_object.addWidget(q_object)

    # QLayout -> QWidget
    elif isinstance(q_object, QtGui.QLayout) and isinstance(parent_object, QtGui.QWidget):
      self._debug('Adding QLayout (' + handle + ') to QWidget (' + parent + ')')
      parent_object.setLayout(q_object)

    # QLayout -> QLayout (Default case when no parent is supplied)
    elif isinstance(q_object, QtGui.QLayout) and isinstance(parent_object, QtGui.QLayout):
      self._debug('Adding QLayout (' + handle + ') to (' + parent + ')')
      parent_object.addLayout(q_object)

    else:
      print 'peacockWarning ...'

    return self._objects[handle]

  ##
  # Connect a signal to a callback (public)
  # @param signal_name The signal name contained within a Peacock object (_signal_<name>) or a Qt.Signal object
  # @param callback_name The handle associated with the QObject added via addObject that
  #                      has a _callback<name> method defined in the class or callable method.
  # @param search_children Toggle the searching of child MooseWidget objects (optional, default is True)
  def connectSignal(self, signal_name, callback_name, search_children = True ):

    # Set signal object
    signal = signal_name
    if isinstance(signal, str):
      signal = self.signal(signal, search_children)

    if not isinstance(signal, QtCore.Signal):
      peacockError('The supplied signal must be valid signal name or a QtCore.Signal object')

    # Get method
    callback = callback_name
    if isinstance(callback, str):
      callback = self.callback(callback, search_children)

    if not hasattr(callback, '__call__'):
      peacockError('The supplied callback must be a valid callback name or a callable function')

    signal.connect(callback)

  ##
  # Run object setup methods (public)
  #
  # When this method is executed, it should be done in the constructor of an object inheriting
  # from MooseWidget, the following is done:
  #   (1) If a method named "initialSetup" exists, it is called.
  #   (2) If a method named "_setup<handle>" exists it is called, where handle is the name given to
  #       the object when adding it via addObject.
  #   (3) If a _setup<handle> method does not exist and a method named _callback<handle> does then
  #       an attempt to connect the QObject with the given handle to the callback method. Currently,
  #       QObjects with the following attributes are supported for automatic connection:
  #           - QAbstractButton via 'clicked' attribute
  def setup(self):

    # Iterate through each of the objects stored
    for key, obj in self._objects.iteritems():

      # Call _initialSetup method
      if hasattr(obj, '_initialSetup'):
        self._debug('Executing _initialSetup for ' +  key + ' object')
        obj._initialSetup()

      # Define the setup and callback method names
      setup_name = '_setup' + key
      callback_name = '_callback' + key

      # Execute the setup method if it exists
      self._debug('Checking for ' + setup_name + ' method')
      if hasattr(self, setup_name):
        method = getattr(self, setup_name)
        self._debug('Executing ' + setup_name + ' method')
        method(obj)

      # Otherwise link the callback if it is possible
      elif hasattr(self, callback_name):
        self._debug('Checking for ' + callback_name + ' method')
        if hasattr(obj, 'clicked'):
          method = getattr(self, callback_name)
          obj.clicked.connect(method)
          self._debug('Connecting ' + callback_name + ' to clicked signal of ' +  key + ' object')

  ##
  # Displays the object and signals for this object (public)
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

# protected:

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

  ##
  # Define a message the prints when the debug flag is set to true (protected)
  # @param message The desired debugging message
  def _debug(self, message):
    if property('debug'):
      caller = inspect.stack()[1]
      frame = caller[0]
      info = inspect.getframeinfo(frame)
      print '[' + self.__class__.__name__ + '][' + info.function + ':' + str(info.lineno) +']', message

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
