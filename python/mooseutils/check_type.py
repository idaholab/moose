from message import MOOSE_DEBUG_MODE
from message import mooseError, mooseWarning, mooseDebug, mooseMessage

def check_type(variable, vtype, debug=None, warning=None, message=None):
    """

    """

    if debug and not MOOSE_DEBUG_MODE:
        return

    msg = None
    if isinstance(variable, (list, set)):
        out = [None]*len(variable)
        for i, v in enumerate(variable):
            out[i] = not isinstance(v, vtype)

        if any(out):
            locs = [i for i, o in enumerate(out) if o]
            msg = "The supplied '{}' of values has the incorrect types at the location(s): {}"
            msg = msg.format(type(variable).__name__, locs)

    elif not isinstance(variable, vtype):
        msg = "The supplied variable is of type '{}', but it must be of type '{}'."
        msg = msg.format(type(variable).__name__, vtype.__name__)

    if (msg is not None):
        if warning:
            mooseWarning(msg)
        elif debug:
            mooseDebug(msg)
        elif message:
            mooseMessage(msg)
        else:
            mooseError(msg)

if __name__ == '__main__':

    class Foo(object):

        @check(int)
        def method(bar):
            return bar

    f = Foo()
    f.method('str')
