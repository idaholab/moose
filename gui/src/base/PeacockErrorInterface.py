from PySide import QtCore, QtGui

##
# Class for providing a peacockError method capable of producing a dialog
# @param kwargs (optional) Key, value pairings (see below)
#
# Optional keyword, value:
#   testing
#   True | <False>
#   If set to true, no error dialog or screen printing occurs. The message is
#   simply stored. It is accessible through getLastErrorMessage()
class PeacockErrorInterface(object):

  ## Constructor (public)
  def __init__(self, **kwargs):

    # Initialize all member variables
    self._all_error_messages = []
    self._last_error_message = None
    self._error_dialog = None
    self._has_dialog = False

    # Create the error dialog if the object is a QWidget
    if isinstance(self, QtGui.QWidget):
      self._error_dialog = QtGui.QErrorMessage(self)
      self._has_dialog = True


  ##
  # Produce an error message (public)
  # If the parent object is a QWidget this will create a dialog error
  # message and/or print the message to the screen. It also stores
  # the message for access via getLastErrorMessage and getAllErrorMessages
  #
  # @param args The error message, all arguments are collected into a single
  #             error message (e.g., peacockError('This', 'is', str(True)))
  # @param kwargs Optional flags to control output
  #
  # Optional Keyword Arguments:
  #   dialog
  #   True | <False>
  #   If true and the Object is a QWidget, an error dialog is created
  #
  #   screen
  #   <True> | False
  #   If true (the default) the error message is printed to the screen
  def peacockError(self, *args, **kwargs):

    # Build and store the message
    message = ' '.join(args)
    self._last_error_message = message
    self._all_error_messages.append(message)

    # Create the dialog
    if self._has_dialog and kwargs.pop('dialog', False):
      self._error_message.showMessage(message)

    # Print to the screen
    if kwargs.pop('screen', True):
      print message

  ##
  # Produce a warning dialog
  # @param args The error message, all arguments are collected into a single
  #             error message (e.g., peacockError('This', 'is', str(True)))
  # @param kwargs Optional flags to control output
  #
  # Optional Keyword Arguments:
  #   cancel
  #   True | <False>
  #   If true a cancel button is included in the warning
  #
  #   dialog
  #   <True> | False
  #   If true and the Object is a QWidget, an error dialog is created
  def peacockWarning(self, *args, **kwargs):
    message = ' '.join(args)

    # Show a dialog box
    if self._has_dialog and kwargs.pop('dialog', True):
      msg_box = QtGui.QMessageBox()
      msg_box.setIcon(QtGui.QMessageBox.Warning)

      # Adds cancel button
      if kwargs.pop('cancel', False):
        msg_box.setStandardButtons(QtGui.QMessageBox.Cancel | QtGui.QMessageBox.Ok)

      # Create the box
      msg_box.setText(message)
      ret = msg_box.exec_()

      # Return False if Cancel was pressed, True otherwise
      if ret == QtGui.QMessageBox.Cancel:
        return False
      else:
        return True

    # Without a dialog, just print the message
    else:
      print message
      return True

  ##
  # Retrieve the last error message (public)
  # @return A string containing the last error message
  def getLastErrorMessage(self):
    return self._last_error_message

  ##
  # Retrieve the all the error messages (public)
  # @return A list of strings containing the error messages
  def getAllErrorMessages(self):
    return self._all_error_messages

  ##
  # Test that the last error message is the same as the message passed in
  # @param msg The message to test
  # @return True if the last error contains the message supplied
  def testLastErrorMessage(self, msg):
    if self._last_error_message == None:
      return False
    else:
      return msg in self._last_error_message

  ##
  # Test that the an error message occured
  # @param msg The message to test
  # @return True if the supplied message was an error
  def hasErrorMessage(self, msg):
    for err in self._all_error_messages:
      if msg in err:
        return True
    return False
