#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

class InputParameters:
    def __init__(self, *args):
        self.valid = {}
        self.strict_types = {}
        self.desc = {}
        self.substitute = {}
        self.required = set()
        self.private = set()
        self.group = {}

    def addRequiredParam(self, name, *args):
        self.required.add(name)
        self.addParam(name, *args)

    def addParam(self, name, *args):
        if len(args) == 2:
            self.valid[name] = args[0]
        self.desc[name] = args[-1]

    def addRequiredParamWithType(self, name, my_type, *args):
        self.required.add(name)
        self.addParamWithType(name, my_type, *args)

    def addParamWithType(self, name, my_type, *args):
        if len(args) == 3:
            self.valid[name] = args[0]
        self.strict_types[name] = my_type
        self.desc[name] = args[-1]

    def addPrivateParam(self, name, *args):
        self.private.add(name)
        if len(args) == 1:
            self.valid[name] = args[0]

    def addStringSubParam(self, name, substitution, *args):
        self.substitute[name] = substitution
        self.addParam(name, *args)

    def isValid(self, name):
        if name in self.valid and self.valid[name] != None and self.valid[name] != []:
            return True
        else:
            return False

    def __contains__(self, item):
        return item in self.desc

    def __getitem__(self, key):
        return self.valid[key]

    def __setitem__(self, key, value):
        self.valid[key] = value

    ##
    # Adds parameters from another InputParameters object via += operator
    # @param add_params The InputParameters object to merge into the existing object
    def __iadd__(self, add_params):

        # Loop through all possible parameters and perform the correct adding into
        # this InputParameters object
        for key in add_params.keys():
            if add_params.isRequired(key):
                self.addRequiredParam(key, add_params[key], add_params.desc[key])
            elif add_params.isValid(key):
                self.addParam(key, add_params[key], add_params.desc[key])
            else:
                self.addParam(key, add_params.desc[key])

        # Return this InputParameters object
        return self

    def type(self, key):
        if key in self.valid:
            return type(self.valid[key])
        else:
            return None

    def basicTypeHelper(self, value_type):
        if value_type == int:
            return "Integer"
        elif value_type == float:
            return "Real"
        elif value_type == bool:
            return "Boolean"
        else:
            return "String"

    def basicType(self, key):
        if key in self.valid:
            if type(self.valid[key]) == list:
                if len(self.valid[key]) == 0:
                    return 'Array:String'
                else:
                    return 'Array:' + self.basicTypeHelper(type(self.valid[key][0]))
            else:
                return self.basicTypeHelper(type(self.valid[key]))
        else:
            # we don't know any better at this point so we return the most general type
            # self.strict_types might help, but it is not used anywhere
            return 'String'

    def keys(self):
        return set([k for k in self.desc])

    def required_keys(self):
        return self.required

    def valid_keys(self):
        return self.valid

    def substitute_keys(self):
        return self.substitute

    def isRequired(self, key):
        return key in self.required

    def getDescription(self, key):
        return self.desc[key]

    ##
    # Specify a group name for the keys listed
    # @param group The name of the group to create or append
    # @param prop_list The list of property names (keys) to add to the group
    def addParamsToGroup(self, group, prop_list):

        # Check that the group is a string
        if not isinstance(group, str):
            print('ERROR: The supplied group name must be a string')
            return

        # Check that the prop_list is a list
        if not isinstance(prop_list, list):
            print('ERROR: The supplied properties must be supplied as a list')
            return

        # Create the storage for the group if it doesn't exist
        if group not in self.group:
            self.group[group] = []

        # Append the list
        self.group[group] += prop_list

    ##
    # Extract the parameters names (keys) from a group
    # @param group The name of the group to extract keys from
    # @return The list of keys for the given group
    def groupKeys(self, group):
        return self.group[group]

    ##
    # Apply common parameters to parameters for this object
    # @param common The common InputParameters object to apply to these parameters
    def applyParams(self, common):

        if not isinstance(common, InputParameters):
            print('ERROR: Supplied "common" variable must of of type InputParameters')
            return

        # Loop through the valid parameters in the common parameters,
        # if they are not valid in this set, then apply them
        for common_key in common.valid_keys():
            if not self.isValid(common_key):
                self[common_key] = common[common_key]


    def printParams(self):
        for k in self.desc:
            value = ''
            if k in self.valid:
                value = self.valid[k]

            print(k.ljust(20), value)
            print(' '.ljust(20), self.desc[k])
