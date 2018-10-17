"""
The MooseDocs systems raises the following exceptions.
"""
import logging
LOG = logging.getLogger(__name__)
class MooseDocsException(Exception):
    """
    General exception.

    Inputs:
        message[str]: (Required) The exception messages.
        *args: (Optoinal) Any values supplied in *args are automatically applied to the to the
               message with the built-in python format method.
        error[str]: (Optional) Add the error message, within a box.
    """
    def __init__(self, message, *args, **kwargs):
        msg = message.format(*args)
        if kwargs.pop('log', False):
            LOG.exception(msg)
        Exception.__init__(self, msg.encode('utf-8'))
