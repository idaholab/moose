import re

from MarkdownTable import MarkdownTable

class MooseObjectParameterTable(MarkdownTable):
    """
    A class for creating markdown tables from parameter data parsed from MOOSE yaml data.
    """

    PARAMETER_TABLE_COLUMNS = ['name', 'cpp_type', 'default', 'description']
    PARAMETER_TABLE_COLUMN_NAMES = ['Name', 'Type', 'Default', 'Description']

    def __init__(self, **kwargs):
        super(MooseObjectParameterTable, self).__init__(*self.PARAMETER_TABLE_COLUMN_NAMES)
        self._parameters = []

    def addParam(self, param):
        """
        Add a parameter to the table.

        Args:
            param[dict]: A parameter dict() extracted from MOOSE yaml data.
        """

        self._parameters.append(param)

        items = []
        for key in self.PARAMETER_TABLE_COLUMNS:
            items.append(self._formatParam(param[key], key, param['cpp_type']))

        self.addRow(*items)

    def _formatParam(self, param, key, ptype):
        """
        Convert the supplied parameter into a format suitable for output.

        Args:
            param[str]: The value of the parameter.
            key[str]: The current key.
            ptype[str]: The C++ type for the parameter.
        """

        # Make sure that supplied parameter is a string
        param = str(param).strip()

        # The c++ types returned by the yaml dump are raw and contain "allocator" stuff. This script attempts
        # to present the types in a more readable fashion.
        if key == 'cpp_type':
            # Convert std::string
            string = 'std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char> >'
            param = param.replace(string, 'std::string')

            # Convert vectors
            param = re.sub(r'std::__1::vector\<(.*),\sstd::__1::allocator\<(.*)\>\s\>', r'std::vector<\1>', param)
            param = '`' + param + '`'

        elif key == 'default':
            if ptype == 'bool':
                param = repr(bool(param))

        return param
