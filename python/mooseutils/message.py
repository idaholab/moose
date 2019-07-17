#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html
import os
import traceback
from mooseutils import colorText

try:
    from PyQt5 import QtWidgets, QtCore
    MOOSE_USE_QT5 = True
except:
    MOOSE_USE_QT5 = False

"""
Global for enabling/disabling debug mode.
"""
MOOSE_DEBUG_MODE = os.environ.get("MOOSE_PYTHON_DEBUG_MODE") == "1"

"""
Global for enabling/disabling testing mode.
"""
MOOSE_TESTING_MODE = False

if MOOSE_USE_QT5:
    class MessageEmitter(QtCore.QObject):
        message = QtCore.pyqtSignal(str, str)

        def write(self, msg, color):
            if not self.signalsBlocked():
                self.message.emit(str(msg), str(color))

    messageEmitter = MessageEmitter()

def mooseMessage(*args, **kwargs):
    """
    A generic message function.

    Args:
        args[tuple]: Comma separated items to be printed, non strings are converted with 'repr'.

    Kwargs:
        error[bool]: (Default: False) When True and 'dialog=True' the the "Critical" icon is included with the message.
        warning[bool]: (Default: False) When True and 'dialog=True' the the "Critical" icon is included with the message.
        traceback[bool]: (Default: False) When True the stack trace is printed with the message.
        dialog[bool]: (Default: False) When true a QDialog object is created, the error will still print to the console as well.
        color[str]: (Default: None) Add the bash color string to the message (see colorText).
        debug[bool]: (Default: False) Print the message only if tools.MOOSE_DEBUG_MODE = True.
        test[bool]: FOR TESTING ONLY! (Default: False) When True the QDialog is not executed, just returned.
        indent[int]: Number of levels to indent (2 spaces are applied to each level)
    """

    # Grab the options
    error = kwargs.pop('error', False)
    warning = kwargs.pop('warning', False)
    trace = kwargs.pop('traceback', False)
    dialog = kwargs.pop('dialog', False)
    color = kwargs.pop('color', None)
    test = kwargs.pop('test', False)
    indent = kwargs.pop('indent', 0)

    # Build the message
    message = []
    for arg in args:
        if not isinstance(arg, str):
            message.append(repr(arg))
        else:
            message.append(arg)
    message = '{}{}'.format(' '*2*indent, ' '.join(message))

    # Show a dialog box
    if MOOSE_USE_QT5 and dialog and not MOOSE_TESTING_MODE:
        box = QtWidgets.QMessageBox()
        box.setText(message)

        if warning:
            box.setIcon(QtWidgets.QMessageBox.Warning)
        elif error:
            box.setIcon(QtWidgets.QMessageBox.Critical)

        if test:
            return box
        box.exec_()

    # Emit the message to any listeners
    if MOOSE_USE_QT5:
      messageEmitter.write(message, color)

    # Print the message to screen
    if color:
        message = colorText(message, color)
    print(message)
    # Show the traceback
    if trace and MOOSE_USE_QT5:
        traceback.print_stack()
        stack = ''.join(traceback.format_stack())
        messageEmitter.write(stack, color)


def mooseError(*args, **kwargs):
    """
    A mooseMessage setup to produce an error.
    """
    return mooseMessage('ERROR\n', *args, error = kwargs.pop('error', True),
                                          color = kwargs.pop('color', 'RED'),
                                          traceback = kwargs.pop('traceback', True),
                                          **kwargs)

def mooseWarning(*args, **kwargs):
    """
    A mooseMessage setup to produce a warning.
    """
    return mooseMessage('WARNING\n', *args, warning = kwargs.pop('warning', True),
                                            color = kwargs.pop('color', 'YELLOW'), **kwargs)


def mooseDebug(*args, **kwargs):
    """
    A mooseMessage that only appears with the global MOOSE_DEBUG_MODE = True or debug=True passed directly.
    """
    if kwargs.pop('debug', MOOSE_DEBUG_MODE):
        return mooseMessage(*args, color=kwargs.pop('color', 'CYAN'), **kwargs)
