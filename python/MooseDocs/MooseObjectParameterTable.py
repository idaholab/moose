from collections import OrderedDict
from markdown.util import etree
import re

class MooseObjectParameterTable(object):
  """
  A class for creating markdown tables from parameter data parsed from MOOSE yaml data.
  """

  # List of parameters to render with their corresponding pretty name.
  PARAMETER_ROW_IDENTIFIERS = [('name',     'Name'),
                               ('cpp_type', 'Type'),
                               ('default',  'Default'),
                               ('description', 'Description')]

  def __init__(self, **kwargs):
    self._rows = []

  def __nonzero__(self):
    return len(self._rows) > 0

  def addParam(self, param):
    """
    Add a parameter to the table.

    Args:
      param[dict]: A parameter dict() extracted from MOOSE yaml data.
    """

    ordered_identifiers = OrderedDict()
    for identifier, pretty_text in self.PARAMETER_ROW_IDENTIFIERS:
      ordered_identifiers[pretty_text] = self._formatParam(param[identifier], identifier, param['cpp_type'])
    self._rows.append(ordered_identifiers)

  def html(self):
    """
    Return html for the div including special class names indicating that this is a parameter table.
    All formatting will be done using CSS
    """

    # Create the parent div
    param_div = etree.Element('div')
    param_div.set('class', 'moose-object-param-table')

    # Create the div containing the header
    header_div = etree.SubElement(param_div, 'div')
    header_div.set('class', 'param-header')
    for header_text in ['Information', 'Description']:
      single_header = etree.SubElement(header_div, 'div')
      single_header.set('class', 'param-header-' + header_text.lower())
      p = etree.SubElement(single_header, 'p')
      p.set('class', 'param-header-p' + header_text.lower())
      p.text = header_text

    # Create Detailed section
    for idx, row in enumerate(self._rows):
      detail_div = etree.SubElement(param_div, 'div')

      # stripe every other div
      if idx in range(len(self._rows))[1::2]:
        detail_div.set('class', 'param-detailed-odd')
      else:
        detail_div.set('class', 'param-detailed-even')

      # create two divs, one for identifiers and one for description
      identifier_div = etree.SubElement(detail_div, 'div')
      identifier_div.set('class', 'param-detailed-identifier')

      description_div = etree.SubElement(detail_div, 'div')
      description_div.set('class', 'param-detailed-description')

      # fill in all the details from the ordered_dict based on identifier
      for identifier, value in row.iteritems():
        if identifier.lower() == 'description':
          sub_div = etree.SubElement(description_div, 'div')
        else:
          sub_div = etree.SubElement(identifier_div, 'div')
        sub_div.set('class', 'param-sub-detailed-' + identifier.lower())

        # create identifier text
        id_div = etree.SubElement(sub_div, 'div')
        id_div.set('class', 'param-sub-id-' + identifier.lower())
        p = etree.SubElement(id_div, 'p')
        p.set('class', 'param-sub-detailed-p-id-' + identifier.lower())
        p.text = identifier

        # create value text
        value_div = etree.SubElement(sub_div, 'div')
        value_div.set('class', 'param-sub-value-' + identifier.lower())
        p = etree.SubElement(value_div, 'p')
        p.set('class', 'param-sub-detailed-p-value-' + identifier.lower())
        p.text = value

    return param_div

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

      param = re.sub(r'std::vector\<(.*),\sstd::allocator\<(.*)\>\s\>',  r'std::vector<\1>', param)
      param = '`' + param + '`'

    elif key == 'default':
      if ptype == 'bool':
        param = repr(bool(param))

    return param
