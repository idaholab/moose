"""
The MooseDocs systems raises the following exceptions.
"""
from box import box
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
        err = kwargs.pop('error', None)
        msg = message.format(*args)
        if err is not None:
            msg += u'\n\n{}'.format(box(err))
        Exception.__init__(self, msg.encode('utf-8'))

class TokenizeException(MooseDocsException):
    """
    Exception for reporting problems during tokenization, this should be called from within
    the 'createToken' method of TokenComponent objects.
    """
    pass

class RenderException(MooseDocsException):
    """
    Exception for reporting problem during rendering, this should be called from within the
    create methods (e.g., 'createHTML', 'createLatex') of RenderComponent objects.
    """
    def __init__(self, info, message, *args):
        MooseDocsException.__init__(self, message, *args)
        self.info = info
