import copy

class ParameterInfo(object):
    """
    Holds the information for a parameter
    """
    def __init__(self, parent, name):
        self.value = ""
        self.user_added = False
        self.name = name
        self.required = False
        self.cpp_type = "string"
        self.group_name = "Main"
        self.description = ""
        self.default = ""
        self.parent = parent
        self.comments = ""
        self.options = []
        self.set_in_input_file = False

    def setFromData(self, data):
        """
        Sets this attributes from a Json dict.
        Input:
            data[dict]: This is the dict description of the parameter as read from the JSON dump.
        """
        self.default = data.get("default", "")
        if self.default is None:
            self.default = ""
        self.cpp_type = data["cpp_type"]
        self.description = data["description"]
        self.group_name = data["group_name"]
        if not self.group_name:
            self.group_name = "Main"
        self.required = data["required"] == "Yes"
        self.name = data["name"]
        self.options = data.get("options", "")
        if self.options:
            self.options = self.options.strip().split()
        else:
            self.options = []

        if self.cpp_type == "bool":
            if self.default == "0":
                self.default = "false"
            elif self.default == "1":
                self.default = "true"
            elif not self.default:
                self.default = "false"
        self.value = self.default

    def copy(self, parent):
        """
        Copies this ParameterInfo to a new one.
        Input:
            parent[BlockInfo]: Parent of the new ParameterInfo
        Return:
            ParameterInfo: The copied parameter
        """
        new = copy.copy(self)
        new.parent = parent
        new.comments = ""
        return new

    def needsQuotes(self):
        """
        Check whether we need to write out quotes around this parameter value.
        Return:
            bool
        """
        return (self.isVectorType() or
            "Point" in self.cpp_type or
            self.user_added or
            ("basic_string" in self.cpp_type and self.name == "value") or
            ' ' in self.value or
            ';' in self.value or
            '=' in self.value or
            '\n' in self.value
            )

    def isVectorType(self):
        """
        Check whether this is a vector type.
        Return:
            bool
        """
        if ('vector' in self.cpp_type or
            'Vector' in self.cpp_type or
            'MultiMooseEnum' == self.cpp_type):
            return True
        return False

    def inputFileValue(self):
        """
        Return the string that should be written to the input file.
        Some values needs single quotes while others do not.
        """
        if self.needsQuotes() and (self.value or self.user_added):
            return "'%s'" % self.value
        else:
            return str(self.value)

    def toolTip(self):
        return self.description

    def dump(self, o, indent=0, sep='  '):
        o.write("%sName: %s\n" % (indent*sep, self.name))
        o.write("%sValue: %s\n" % (indent*sep, self.value))
        o.write("%sDefault: %s\n" % (indent*sep, self.default))
        o.write("%sUser added: %s\n" % (indent*sep, self.user_added))
        o.write("%sRequired: %s\n" % (indent*sep, self.required))
        o.write("%sCpp_type: %s\n" % (indent*sep, self.cpp_type))
        o.write("%sGroup: %s\n" % (indent*sep, self.group_name))
        o.write("%sDescription: %s\n" % (indent*sep, self.description))
        o.write("%sComments: %s\n" % (indent*sep, self.comments))
