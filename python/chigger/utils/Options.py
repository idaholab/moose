#pylint: disable=missing-docstring
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

#!/usr/bin/env python2
from __future__ import print_function
import sys
import textwrap
from collections import OrderedDict
import traceback
try:
    import terminaltables
    HAS_TERMINAL_TABLES = True
except ImportError:
    HAS_TERMINAL_TABLES = False
import mooseutils

class Option(object):
    """
    Storage container for an "option" that can be type checked.
    """

    ANY = 12345

    def __init__(self, *args, **kwargs):

        # Stored the supplied values
        # NOTE: The default and value are private to keep them from the type checking being
        # nullified if the values are just set directly.
        if len(args) == 2:
            self.name = args[0]
            self.__value = None
            self.doc = args[1]
            self.__default = None
        elif len(args) == 3:
            self.name = args[0]
            self.__value = args[1]
            self.doc = args[2]
            self.__default = self.__value
        else:
            raise Exception("Wrong number of arguments, must supply 2 or 3 input arguments.")

        # Extract optional settings
        self.vtype = kwargs.pop('vtype', type(self.__value))
        self.allow = kwargs.pop('allow', [])

        # Check that allow is correct type
        if not isinstance(self.allow, list):
            mooseutils.mooseWarning('The allow option must be supplied as a list.')

        # Check the allowed list contains the correct types
        else:
            for i in range(len(self.allow)):
                try:
                    if not isinstance(self.allow[i], self.vtype) and self.vtype != Option.ANY:
                        self.allow[i] = eval('{}({})'.format(self.vtype.__name__,
                                                             str(self.allow[i])))

                except: #pylint: disable=bare-except
                    msg = 'The type provided, {}, does not match the type of the allowed ' + \
                          'values, {} for the {} option.'
                    mooseutils.mooseWarning(msg.format(self.vtype.__name__,
                                                       type(self.allow[i]).__name__, self.name))
                    return

        # Try to set the value using the set method to test the type and if it is allowed
        if (self.__value != None) and not isinstance(self.__value, Options):
            self.set(self.__value)

    def unset(self):
        """
        Set the value of the option to None.
        """
        self.__value = None

    def set(self, value):
        """
        Set the value of this option, with type checking.

        Inputs:
            value: The value to set the option to.
        """
        self.__setValue(value)

    def get(self):
        """
        Get the value of this option.
        """
        return self.__value

    def setDefault(self, value):
        """
        Set the value of this option along with the default.

        Inputs:
            value: The value to set the option and its default to.
        """
        self.__setValue(value, set_default=True)

    def getDefault(self):
        """
        Get the default value for this option.
        """
        return self.__default

    def __setValue(self, value, set_default=False):
        """
        Set the value of the option with type checking.

        Inputs:
            set_default[bool]: (default: False) When true the value passed in is also set to the
                               default.
        """

        # None is always allowed
        if value is None:
            self.unset()
            return

        # If the Option is storing another Set of options and is passed a dict(), then
        # loop through the dictionary and update each option in the set of options.
        if (self.vtype is Options) and isinstance(value, dict):
            for k, v in value.iteritems():
                self.__value[k] = v

        else:
            if not isinstance(value, self.vtype):

                # Check if we can convert (e.g., int->float)
                try:
                    value = eval(self.vtype.__name__ + '(' + str(value) +')')
                except: #pylint: disable=bare-except
                    msg = '{} must be of type {} but {} provided.'
                    mooseutils.mooseWarning(msg.format(self.name, self.vtype.__name__,
                                                       type(value).__name__))
                    value = None

            # Check that the value is allowed
            if self.allow and (value != None) and (value not in self.allow):
                msg = 'Attempting to set {} to a value of {} but only the following are allowed: {}'
                mooseutils.mooseWarning(msg.format(self.name, value, self.allow))
                value = None

            self.__value = value

            if set_default:
                self.__default = value



class Options(object):
    """
    A warehouse for creating and storing options
    """

    def __init__(self):
        self.__options = OrderedDict()

    def keys(self):
        """
        List of option names
        """
        return self.__options.keys()

    def raw(self, name):
        """
        Return the option class.

        Inputs:
            name[str]: The name of the Option to retrieve
        """
        return self.__options[name]

    def pop(self, name, default=None):
        """
        Remove an Option object from the available options. (public)

        Inputs:
            name[str]: The name of the Option to retrieve
        """
        if not self.hasOption(name):
            return default
        else:
            option = self.__options.pop(name)
            return option.get() #pylint: disable=no-member

    def hasValidOption(self):
        """
        Test if any option is valid within the entire set of options.
        """
        for key in self.__options.keys():
            if self.isOptionValid(key):
                return True
        return False

    def isOptionValid(self, name):
        """
        Test if the given option is valid (i.e., !None). (public)

        Inputs:
            name[str]: The name of the Option to retrieve
        """
        return self.hasOption(name) and (self[name] != None)

    def isOptionDefault(self, name):
        """
        Test if the options is currently set to the default value.

        Inputs:
            name[str]: The name of the Option to retrieve
        """
        return self.hasOption(name) and \
               (self.__options[name].get() == self.__options[name].getDefault())

    def setDefault(self, name, value):
        """
        Set the value and the default to the supplied value.

        Inputs:
            name[str]: The name of the Option to retrieve
            value: The value to set the option to
        """
        self.__options[name].setDefault(value)

    def __setitem__(self, name, value):
        """
        Overload for setting value of an option with [].

        Inputs:
            name[str]: The name of the Option to retrieve
            value: The value to set the option to
            **kwargs: Key, value pairs are passed to the Option object.
        """

        # Check that the option exists
        if not self.hasOption(name):
            mooseutils.mooseWarning('No option with the name:', name)

        # Set option to the given value, type checking occurs in the Option object
        else:
            self.__options[name].set(value)

    def __getitem__(self, name):
        """
        Overload for accessing the parameter value by name with []

        Inputs:
            name[str]: The name of the Option to retrieve
        """
        #@todo warning
        if name not in self.__options:
            print('No option with the name:', name)
            print(self.__options.keys())
            traceback.print_stack()
            sys.exit()
            return None

        # Return None if the value is none, without this bool
        # checks on the value do not seem to work.
        if self.__options[name].get() is None:
            return None
        return self.__options[name].get()

    def hasOption(self, name):
        """
        Test that the option exists.

        Inputs:
            name[str]: The name of the Option to retrieve
        """
        return name in self.__options

    def add(self, *args, **kwargs):
        """
        Add a new option to the warehouse

        Inputs:
            name[str]: The name of the option, used to access and set its value
            value: The value to set the property too (i.e., the default value)
            doc[str]: The documentation string

        Optional Key, value Pairs
            vtype: The type of the value
            allowed[list]: The allowed values
        """
        if args[0] in self.__options:
            mooseutils.mooseWarning('A parameter with the name', args[0], 'already exists.')
            return

        self.__options[args[0]] = Option(*args, **kwargs)

    def update(self, options=None, **kwargs):
        """"
        Update the options given key, value pairs

        To enable unused warning, include 'warn_unused=True'
        """

        # Unused options
        changed = False
        unused = set()
        warn = kwargs.pop('warn_unused', False)

        # Update from Options object
        if isinstance(options, Options):
            for key in options.keys():
                if self.hasOption(key):
                    if options.isOptionValid(key):
                        changed = True
                        self[key] = options[key]
                else:
                    unused.add(key)

        # Update from kwargs
        for k, v in kwargs.iteritems():
            if k in self.__options:
                changed = True
                self[k] = v
            else:
                unused.add(k)

        # Warning for unused @todo warning
        if warn and len(unused) > 0:
            msg = 'The following settings where not recognized:'
            for key in unused:
                msg += ' '*4 + key
            mooseutils.mooseWarning(msg)

        return changed

    def __iadd__(self, options):
        """
        Append another Option objects options into this container.

        Inputs:
           options[Options]: An instance of an Options object to append.
        """
        for key in options.keys():
            self.__options[key] = options.raw(key)
        return self

    def __str__(self):
        """
        Allows print to work with this class.
        """
        return self.string()

    def string(self, **kwargs):
        """
        Functions to output the options to a string
        """
        if not HAS_TERMINAL_TABLES:
            return self.__class__.__name__

        width = kwargs.pop('width', 120)

        def build(options, tables, title=None):
            """
            Helper for building terminal table
            """
            data = [['Option', 'Value', 'Type', 'Allowed', 'Description']]
            for key in options.keys():
                opt = options.raw(key)
                if isinstance(opt.vtype, tuple):
                    vtype = repr([v.__name__ for v in opt.vtype])
                else:
                    vtype = opt.vtype.__name__
                if not isinstance(opt.get(), Options):
                    data.append([opt.name, repr(opt.get()), vtype, repr(opt.allow), opt.doc])
                else:
                    data.append([opt.name, 'Options', vtype, repr(opt.allow), opt.doc])
                    build(opt.get(), tables, title=opt.name)

            table = terminaltables.SingleTable(data, title=title)
            n = sum(table.column_widths[:-2])
            for i in range(len(table.table_data)):
                table.table_data[i][-2] = '\n'.join(textwrap.wrap(table.table_data[i][-2], 24))
                table.table_data[i][-1] = '\n'.join(textwrap.wrap(table.table_data[i][-1],
                                                                  width-(n+24)))
            tables.append(table.table)

        tables = []
        build(self, tables)
        return '\n\n'.join(reversed(tables))

    def toScriptString(self, **kwargs):
        """
        Takes an Options object and returns a string for building python scripts.

        Inputs:
            kwargs: Key, value pairs provided will replace values in options with the string given
                    in the value.
        """
        output = []
        sub_output = dict()
        for key in self.keys():
            opt = self[key]

            if isinstance(opt, Options):
                items, _ = opt.toScriptString()
                if items:
                    sub_output[key] = items

            elif not self.isOptionDefault(key):
                if key in kwargs:
                    r = kwargs[key]
                else:
                    r = repr(opt)
                output.append('{}={}'.format(key, r))

        return output, sub_output
