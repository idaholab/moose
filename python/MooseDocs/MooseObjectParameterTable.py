import re
from markdown.util import etree

class MooseObjectParameterTable(object):
  """
  A class for creating markdown tables from parameter data parsed from MOOSE yaml data.
  """

  INNER = [('name', 'Name:'), ('cpp_type', 'Type:'), ('default', 'Default:')]

  def __init__(self, display_type='collapsible', **kwargs):
    self._parameters = []
    self._display_type = display_type

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
    Return html for the table including special class name indicating that this is a parameter table.
    """
    if self._display_type == 'collapsible':
      el = self.htmlCollapsible()
    else:
      el = self.htmlTable()
    return el

  def htmlCollapsible(self):
    """
    Shows parameters as collapsible items.
    """

    ul = etree.Element('ul')
    ul.set('class', "collapsible moose-parameter-collapsible")
    ul.set('data-collapsible', "expandable")
    for param in self._parameters:
      li = etree.SubElement(ul, 'li')
      header = etree.SubElement(li, 'div')
      header.set('class', "collapsible-header")
      i = etree.SubElement(header, 'i')
      i.text = 'input'
      i.set('class', 'material-icons')

      header.text = param['name']

      default = self._formatParam(param, 'default')
      if default:
        span = etree.SubElement(header, 'span')
        span.text = '(Default: {})'.format(default)

      desc = etree.SubElement(li, 'div')
      desc.set('class', "collapsible-body")
      p = etree.SubElement(desc, 'p')
      p.text = '{}<br>Type: {}'.format(param['description'], self._formatParam(param, 'cpp_type'))

    return ul

  def htmlTable(self):
    """
    Creates a table for displaying parameters
    """

    outer_table = self.element('table', cls='moose-parameter-table-outer')

    # Header
    header_tr = etree.SubElement(outer_table, 'tr')
    header_tr.append(self.element('th', 'Information', cls='moose-parameter-table-info'))
    header_tr.append(self.element('th', 'Description', cls='moose-parameter-table-desc'))

    for param in self._parameters:
      param_tr = etree.SubElement(outer_table, 'tr')
      param_td = etree.SubElement(param_tr, 'td')

      # Information
      info_table = etree.SubElement(param_td, 'table')
      info_table.set('class', 'moose-parameter-table-inner')
      for key, name in self.INNER:
        inner_tr = etree.SubElement(info_table, 'tr')
        inner_th = etree.SubElement(inner_tr, 'th')
        inner_th.append(self.element('p', name))

        text = self._formatParam(param[key], key, param['cpp_type'])
        inner_tr.append(self.element('td', text))

      # Description
      param_tr.append(self.element('td', param['description']))

    return outer_table

  @staticmethod
  def element(tag, text=None, cls=None, id=None):
    el = etree.Element(tag)

    if text:
      el.text = text

    if cls:
      el.set('class', cls)

    if id:
      el.set('id', id)

    return el

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
