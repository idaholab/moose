import re
from markdown.util import etree

class MooseObjectParameterTable(object):
  """
  A class for creating markdown tables from parameter data parsed from MOOSE yaml data.
  """

  INNER = [('name', 'Name:'), ('cpp_type', 'Type:'), ('default', 'Default:')]

  def __init__(self, **kwargs):
    self._parameters = []

  def __nonzero__(self):
    return len(self._parameters) > 0

  def addParam(self, param):
    """
    Add a parameter to the table.

    Args:
      param[dict]: A parameter dict() extracted from MOOSE yaml data.
    """

    self._parameters.append(param)

  def html(self):
    """
    Return html containing collapsible items.
    """

    ul = etree.Element('ul')
    ul.set('class', "collapsible")
    ul.set('data-collapsible', "expandable")
    for param in self._parameters:
      li = etree.SubElement(ul, 'li')
      header = etree.SubElement(li, 'div')
      header.set('class', "collapsible-header")

      header_row = etree.SubElement(header, 'div')
      header_row.set('class', 'row')

      header_name = etree.SubElement(header_row, 'div')
      header_name.set('class', 'moose-parameter-name col l4')
      header_name.text = param['name']

      default = self._formatParam(param, 'default').strip()
      if default:
        default_span = etree.SubElement(header_name, 'span')
        default_span.set('class', 'moose-parameter-header-default')
        default_span.text = ' ({})'.format(default)

      description = param['description'].strip()
      if description:
        div = etree.SubElement(header_row, 'div')
        div.set('class', 'moose-parameter-header-description col l8 hide-on-med-and-down')
        div.text = description

      body = etree.SubElement(li, 'div')
      body.set('class', "collapsible-body")

      if description:
        div = etree.SubElement(body, 'div')
        div.set('class', 'moose-parameter-description')
        div.text = description

      div = etree.SubElement(body, 'div')
      div.set('class', 'moose-parameter-default')
      if default:
        div.text = 'Default: {}'.format(default)
      else:
          div.text = 'Default: None'

      div = etree.SubElement(body, 'div')
      div.set('class', 'moose-parameter-type')
      div.text = 'Type: {}'.format(self._formatParam(param, 'cpp_type'))

    return ul

  def _formatParam(self, parameter, key):
    """
    Convert the supplied parameter into a format suitable for output.

    Args:
      param[str]: The value of the parameter.
      key[str]: The current key.
      ptype[str]: The C++ type for the parameter.
    """

    # Make sure that supplied parameter is a string
    ptype = parameter['cpp_type']
    param = str(parameter[key]).strip()

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
