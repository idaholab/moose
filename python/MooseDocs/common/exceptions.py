"""
The MooseDocs systems raises the following exceptions.
"""

class MooseDocsException(Exception):
    """
    General exception.

    Inputs:
        message[str]: (Required) The exception messages.
        *args: (Optoinal) Any values supplied in *args are automattically applied to the to the
               message with the built-in python format method.
    """
    def __init__(self, message, *args):
        Exception.__init__(self, message.format(*args))

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
