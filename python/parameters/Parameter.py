#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html
import textwrap
import inspect
import logging
import mooseutils
LOG = logging.getLogger(__file__)

class Parameter(object):
    """
    Storage container for an "param" that can be type checked, restricted, and documented.

    The meta data within this object is designed to be immutable, the only portion of this class
    that can be changed (without demangling) is the assigned value and the default, via the
    associated setter methods.

    Inputs:
        name[str]: The name of the option.
        default[]: The default value, if "vtype" is set the type must match.
        doc[str]: A documentation string, which is used in the option dump.
        vtype[type]: The python type that this option is to be restricted.
        allow[tuple]: A tuple of allowed values, if vtype is set the types within must match.
        size[int]: Defines the size of an array, setting size will automatically set the array flag.
        array[bool]: Define the option as an "array", which if 'vtype' is set restricts the values
                     within the tuple to match types.
        required[bool]: Define if the option is required; see InputParameters.py
        private[bool]: Define if the options is private; see InputParameters.py. A parameter name
                       that starts with and underscore is assumed to be private
        verify[tuple]: Define a custom verify function and error message. The first item must
                       be a callable function with a single argument, the second item must be a str.
    """
    def __init__(self, name, default=None, doc=None, vtype=None, allow=None, size=None,
                 array=False, required=False, private=None, verify=None):

        # Force vtype to be a tuple to allow multiple types to be defined
        if isinstance(vtype, type):
            vtype = (vtype,)

        self.__name = name         # option name
        self.__value = None        # current value
        self.__default = default   # default value
        self.__vtype = vtype       # option type
        self.__allow = allow       # list of allowed values
        self.__doc = doc           # documentation string
        self.__array = array       # create an array
        self.__size = size         # array size
        self.__required = required # see validate()
        self.__verify = verify     # verification function
        self.__set_by_user = False # flag indicating if the parameter was set after construction

        if not isinstance(self.__name, str):
            msg = "The supplied 'name' argument must be a 'str', but {} was provided."
            raise TypeError(msg.format(type(self.__name)))

        # private option, must be after name to allow startswith to work without error
        self.__private = private if (private is not None) else self.__name.startswith('_')

        if (self.__doc is not None) and (not isinstance(self.__doc, str)):
            msg = "The supplied 'doc' argument must be a 'str', but {} was provided."
            raise TypeError(msg.format(type(self.__doc)))

        if (self.__vtype is not None) and (any(not isinstance(v, type) for v in self.__vtype)):
            msg = "The supplied 'vtype' argument must be a 'type', but {} was provided."
            raise TypeError(msg.format(type(self.__vtype)))

        if (self.__allow is not None) and (not isinstance(self.__allow, tuple)):
            msg = "The supplied 'allow' argument must be a 'tuple', but {} was provided."
            raise TypeError(msg.format(type(self.__allow)))

        if (self.__vtype is not None) and (self.__allow is not None):
            for value in self.__allow:
                if not isinstance(value, self.__vtype):
                    msg = "The supplied 'allow' argument must be a 'tuple' of {} items, but a {} " \
                            "item was provided."
                    raise TypeError(msg.format(self.__vtype, type(value)))

        if (self.__size is not None) and (not isinstance(self.__size, int)):
            msg = "The supplied 'size' argument must be a 'int', but {} was provided."
            raise TypeError(msg.format(type(self.__size)))

        if not isinstance(self.__required, bool):
            msg = "The supplied 'required' argument must be a 'bool', but {} was provided."
            raise TypeError(msg.format(type(self.__required)))

        if not isinstance(self.__private, bool):
            msg = "The supplied 'private' argument must be a 'bool', but {} was provided."
            raise TypeError(msg.format(type(self.__required)))

        if (self.__verify is not None) and (not isinstance(self.__verify, tuple)):
            msg = "The supplied 'verify' argument must be a 'tuple' with callable function and 'str' error message, but {} was provided."
            raise TypeError(msg.format(type(self.__verify)))

        if (self.__verify is not None) and (len(self.__verify) != 2):
            msg = "The supplied 'verify' argument must be a 'tuple' with two items a callable function and 'str' error message, but {} with {} items was provided."
            raise TypeError(msg.format(type(self.__verify), len(self.__verify)))

        if (self.__verify is not None) and (not (inspect.isfunction(self.__verify[0]) or inspect.ismethod(self.__verify[0]))):
            msg = "The first item in the 'verify' argument tuple must be a callable function with a single argument, but {} was provided"
            raise TypeError(msg.format(type(self.__verify[0])))

        if (self.__verify is not None) and (len(inspect.signature(self.__verify[0]).parameters) != 1):
            msg = "The first item in the 'verify' argument tuple must be a callable function with a single argument, but {} was provided that has {} arguments."
            raise TypeError(msg.format(type(self.__verify[0]), len(inspect.signature(self.__verify[0]).parameters)))

        if (self.__verify is not None) and (not isinstance(self.__verify[1], str)):
            msg = "The second item in the 'verify' argument tuple must be a string, but {} was provided"
            raise TypeError(msg.format(type(self.__verify[1])))

        elif self.__size is not None:
            self.__array = True

        if default is not None:
            self.value = default
            self.__set_by_user = False # override self.value setting of this

    @property
    def name(self):
        """Returns the option name."""
        return self.__name

    @property
    def value(self):
        """Returns the option value."""
        return self.__value

    @property
    def default(self):
        """Returns the default value for the option."""
        return self.__default

    @property
    def doc(self):
        """Returns the documentation string."""
        return self.__doc

    @property
    def allow(self):
        """Returns the allowable values for the option."""
        return self.__allow

    @property
    def size(self):
        """Returns the size of the option."""
        return self.__size

    @property
    def vtype(self):
        """Returns the variable type."""
        return self.__vtype

    @property
    def required(self):
        """Returns if the option required state."""
        return self.__required

    @property
    def private(self):
        """Returns if the option private state."""
        return self.__private

    @default.setter
    def default(self, val):
        """Set the default value for this option."""
        if val is None:
            self.__default = None

        else:
            d = self.__check(val)
            if d is not None:
                self.__default = d
                if self.__value is None:
                    self.__value = self.__default

    @value.setter
    def value(self, val):
        """
        Sets the value and performs a myriad of consistency checks.
        """
        if (val is None) and (self.__value is not None):
            self.__value = None
            self.__set_by_user = True
            return

        v = self.__check(val)
        if v is not None:
            self.__value = v
            self.__set_by_user = True

    def isSetByUser(self):
        """Return True if the value has been set after construction."""
        return self.__set_by_user

    def validate(self):
        """Validate that the Parameter is in the correct state."""
        if self.__required and (self.value is None):
            msg = "The Parameter '%s' is marked as required, but no value is assigned."
            LOG.warning(msg, self.name)
            return 1
        return 0

    def toString(self, prefix='', level=0):
        """Create a string of Parameter information."""
        from .InputParameters import InputParameters
        is_sub_option = (self.__vtype is not None) and (InputParameters in self.__vtype) and (self.__value is not None)

        out = [mooseutils.colorText(self.__name, 'LIGHT_YELLOW')]
        if prefix is not None:
            out[0] = '{} | {}{}'.format(out[0], prefix, self.__name) if prefix else out[0]

        if self.__doc is not None:
            wrapper = textwrap.TextWrapper()
            wrapper.initial_indent = ' '*2
            wrapper.subsequent_indent = ' '*2
            wrapper.width = 100
            out += [mooseutils.colorText(w, 'GREY') for w in wrapper.wrap(self.__doc)]

        if is_sub_option:
            out += [self.__value.toString(prefix=self.__name + '_', level=level+1)]

        else:
            out += ['  Value:   {}'.format(repr(self.value))]
            if self.__default is not None:
                out += ['  Default: {}'.format(repr(self.__default))]

            if self.__vtype is not None:
                out += ['  Type(s): {}'.format(tuple([t.__name__ for t in self.__vtype]))]

            if self.__allow is not None:
                wrapper = textwrap.TextWrapper()
                wrapper.initial_indent = '  Allow:   '
                wrapper.subsequent_indent = ' '*len(wrapper.initial_indent)
                wrapper.width = 100 - len(wrapper.initial_indent)
                out += wrapper.wrap(repr(self.__allow))

        return textwrap.indent('\n'.join(out), ' '*2*level)

    def __check(self, val):
        """
        Check that the supplied value is correct.

        This function is used to when setting the default and the value itself.
        """
        if self.__array and not isinstance(val, tuple):
            msg = "'%s' was defined as an array, which require %s for assignment, but a %s was " \
                  "provided."
            LOG.warning(msg, self.name, tuple, type(val))
            return None

        if self.__array:
            for v in val:
                if (self.__vtype is not None) and not isinstance(v, self.__vtype):
                    msg = "The values within '%s' must be of type %s but %s provided."
                    LOG.warning(msg, self.name, self.__vtype, type(v))
                    return None

            if self.__size is not None:
                if len(val) != self.__size:
                    msg = "'%s' was defined as an array with length %s but a value with length %s " \
                          "was provided."
                    LOG.warning(msg, self.name, self.__size, len(val))
                    return None

        else:
            if (self.__vtype is not None) and not isinstance(val, self.__vtype):
                msg = "'%s' must be of type %s but %s provided."
                LOG.warning(msg, self.name, self.__vtype, type(val))
                return None

        # Check that the value is allowed
        if (self.__allow is not None) and (val != None) and (val not in self.__allow):
            msg = "Attempting to set '%s' to a value of %s but only the following are allowed: %s"
            LOG.warning(msg, self.name, val, self.__allow)
            return None

        # Call custom verify function
        if (self.__verify is not None) and (not self.__verify[0](val)):
            msg = "Verify function failed with the given value of {}\n{}"
            LOG.warning(msg.format(val, self.__verify[1]))
            return None

        return val

    def __str__(self):
        """Support print statement on Parameter object."""
        return self.toString()
