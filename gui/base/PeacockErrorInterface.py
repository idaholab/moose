#!/usr/bin/python

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
class PeacockErrorInterface:

  ## Constructor (public)
  def __init__(self, **kwargs):

    # Initialize all member variables
    self._all_error_messages = []
    self._last_error_message = None
    self._error_dialog = None
    self._has_dialog = False

    # Extract the optional arguments
    self._testing = kwargs.pop('testing', False)

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
  #   <True> | False
  #   If true (the default) and the Object is a QWidget, an error dialog is created
  #
  #   screen
  #   <True> | False
  #   If true (the default) the error message is printed to the screen

  def peacockError(self, *args, **kwargs):

    # Build and store the message
    message = ' '.join(args)
    self._last_error_message = message
    self._all_error_messages.append(message)

    # Eat the message if running in testing mode
    if self._testing:
      return

    # Create the dialog
    if self._has_dialog and kwargs.pop('dialog', True):
      self._error_message.showMessage(message)

    # Print to the screen
    if kwargs.pop('screen', True):
      print message

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
