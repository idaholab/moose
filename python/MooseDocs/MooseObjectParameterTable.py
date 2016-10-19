from markdown.util import etree
import logging
import re
log = logging.getLogger(__name__)


from MarkdownTable import MarkdownTable

class MooseObjectParameterTable(MarkdownTable):
  """
  A class for creating markdown tables from parameter data parsed from MOOSE yaml data.
  """

  PARAMETER_TABLE_ROW_NAMES = ['Information', '', 'Description']
  PARAMETER_TABLE_COLUMNS = ['name', 'cpp_type', 'default', 'description']
  PARAMETER_TABLE_COLUMN_NAMES = ['Name:', 'Type:', 'Default:', 'Description']

  def __init__(self, **kwargs):
    self._parameters = []
    self._rows = []

  def addParam(self, param):
    """
    Add a parameter to the table.

    Args:
      param[dict]: A parameter dict() extracted from MOOSE yaml data.
    """

    self._parameters.append(param)

    items = {}

    for idx, key in enumerate(self.PARAMETER_TABLE_COLUMNS):
      items[self.PARAMETER_TABLE_COLUMN_NAMES[idx]] = self._formatParam(param[key], key, param['cpp_type'])
    for key in self.PARAMETER_TABLE_COLUMN_NAMES:
      if key == self.PARAMETER_TABLE_COLUMN_NAMES[0]:
        self._rows.append([self.PARAMETER_TABLE_COLUMN_NAMES[0], items[key], items[self.PARAMETER_TABLE_COLUMN_NAMES[-1]]])
      elif key != self.PARAMETER_TABLE_COLUMN_NAMES[-1]:
        self._rows.append([key, items[key]])

  def html(self):
    """
    Return html for the table including special class name indicating that this is a parameter table.
    """

    table = etree.Element('table')
    table.set('class', 'moose-object-param-table')
    tr = etree.SubElement(table, 'tr')
    for header in self.PARAMETER_TABLE_ROW_NAMES:
      th = etree.SubElement(tr, 'th')
      th.set('class','param-header')
      th.text = header

    for idr, row in enumerate(self._rows):
      tr = etree.SubElement(table, 'tr')
      for idd, d in enumerate(row):
        td = etree.SubElement(tr, 'td')

        # Apply indentifier for first column
        if idd == 0:
          td.set('class', 'row-identifier')

        # Apply stripe class (for 3 rows) when necessary
        if self._canStripe(idr, len(self._rows), 3):
          if td.get('class') is not None:
            td.set('class', ' '.join(['row-even', td.get('class')]))
          else:
            td.set('class', 'row-even')

        # Apply a rowspan for description field only
        if row[0] == self.PARAMETER_TABLE_COLUMN_NAMES[0] and idd == 2:
          td.set('rowspan', '3')
          td.set('valign', 'top')
          if td.get('class') is not None:
            td.set('class', ' '.join(['param-description', td.get('class')]))
          else:
            td.set('class', 'param-description')

        # Add content to <td>
        if isinstance(d, str):
          td.text = d
        else:
          td.append(d)

      # Create another row to simulate a spacer
      if row[0] == self.PARAMETER_TABLE_COLUMN_NAMES[-2]:
        tr = etree.SubElement(table, 'tr')
        tr.set('class', 'param-rowspacer')
        for spacer in self.PARAMETER_TABLE_COLUMN_NAMES:
          td = etree.SubElement(tr, 'td')
          td.set('class', 'row-spacer')

    return table

  def _canStripe(self, current_row, total_rows, iterator):
    """
    returns True if we should set this cell as a stripped cell based on inputs

    Args:
      current_row[int]: The current row we are on
      total_rows[int]: The total number of rows
      iterator[int]: The amount of rows to include in a section
    """
    every_nth = range(total_rows)[::iterator]
    for x in every_nth[::2]:
      if current_row in [i for i in xrange(x, (x + iterator), 1)]:
        return True
    return False

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
