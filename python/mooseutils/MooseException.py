class MooseException(Exception):
    """
    An Exception for MOOSE python applications.
    """

    def __init__(self, *args):
        message = ' '.join([str(x) for x in args])
        Exception.__init__(self, message)
