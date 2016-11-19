import re
import os
import copy
import collections
import logging
log = logging.getLogger(__name__)

from markdown.inlinepatterns import Pattern
from markdown.util import etree
from MooseSyntaxBase import MooseSyntaxBase
from MooseObjectParameterTable import MooseObjectParameterTable
import utils

class MooseObjectSyntax(MooseSyntaxBase):
  """
  Extracts the description from a MooseObject parameters.

  Markdown Syntax:
  !<Keyword> <YAML Syntax> key=value, key1=value1, etc...

  Keywords Available:
    !description - Returns the description added via the 'addClassDescription' method
    !parameters - Returns a set of tables containing the input parameters
    !inputfiles - Returns a set of lists containing links to the input files that use the syntax
    !childobjects - Returns a set of lists containing links to objects that inherit from this class

  "paramaters" Settings:
    display[str]: "table" or "collapsible" to toggle the display type (default: "collapsible"

  """

  RE = r'^!(description|parameters|inputfiles|childobjects)\s+(.*?)(?:$|\s+)(.*)'

  def __init__(self, database=None, repo=None, **kwargs):
    super(MooseObjectSyntax, self).__init__(self.RE, **kwargs)
    self._settings['display'] = "collapsible"

    # Input arguments
    self._input_files = database.inputs
    self._child_objects = database.children
    self._repo = repo
    self._name = None

  def handleMatch(self, match):
    """
    Create a <p> tag containing the supplied description from the YAML dump.
    """

    # Extract match options and settings
    action = match.group(2)
    syntax = match.group(3)

    # Extract Settings
    settings = self.getSettings(match.group(4))

    # Locate description
    node = self._yaml.find(syntax)
    if not node:
      return self.createErrorElement('Failed to locate {} syntax.'.format(syntax))

    # Determine object name
    self._name = node['name'].split('/')[-1]

    if action == 'description':
      el = self.descriptionElement(node, settings)
    elif action == 'parameters':
      el = self.parametersElement(node, settings)
    elif action == 'inputfiles':
      el = self.inputfilesElement(node, settings)
    elif action == 'childobjects':
      el = self.childobjectsElement(node, settings)
    elif action == 'subobjects':
      el = self.subobjectsElement(node, settings)
    return el

  def descriptionElement(self, node, settings):
    """
    Return the class description html element.

    Args:
      node[dict]: YAML data node.
      styles[dict]: Styles from markdown.
    """

    if ('description' not in node) or (not node['description']):
      hidden = any([obj.hidden(node['name']) for obj in self._syntax.itervalues()])
      return self.createErrorElement('Failed to locate class description for {} syntax.'.format(node['name']),warning=hidden)

    # Create the html element with supplied styles
    el = self.applyElementSettings(etree.Element('p'), settings)
    el.text = node['description']
    return el

  def parametersElement(self, node, settings):
    """
    Return table(s) of input parameters.

    Args:
      node[dict]: YAML data node.
      styles[dict]: Styles from markdown.
    """

    # Create the tables (generate 'Required' and 'Optional' initially so that they come out in the proper order)
    tables = collections.OrderedDict()
    tables['Required'] = MooseObjectParameterTable(display_type = settings['display'])
    tables['Optional'] = MooseObjectParameterTable(display_type = settings['display'])

    # Loop through the parameters in yaml object
    for param in node['parameters'] or []:
      name = param['group_name']
      if not name and param['required']:
        name = 'Required'
      elif not name and not param['required']:
        name = 'Optional'

      if name not in tables:
        tables[name] = MooseObjectParameterTable(display_type = settings['display'])
      tables[name].addParam(param)

    el = self.applyElementSettings(etree.Element('div'), settings)
    el.set('id', '#input-parameters')
    el.set('class', 'section scrollspy')
    if any(tables.values()):
      title = etree.SubElement(el, 'h2')
      title.text = 'Input Parameters'
      for key, table in tables.iteritems():
        if table:
          subtitle = etree.SubElement(el, 'h3')
          subtitle.text = '{} {}'.format(key, 'Parameters')
          el.append(table.html())

    return el


  def inputfilesElement(self, node, settings):
    """
    Return the links to input files and child objects.

    Args:
      node[dict]: YAML data node.
      styles[dict]: Styles from markdown.
    """
    # Print the item information
    el = self.applyElementSettings(etree.Element('div'), settings)
    el.set('id', '#input-files')
    el.set('class', 'section scrollspy')
    self._listhelper(node, 'Input Files', el, self._input_files)
    return el

  def childobjectsElement(self, node, settings):
    """
    Return the links to input files and child objects.

    Args:
      node[dict]: YAML data node.
      styles[dict]: Styles from markdown.
    """
    # Print the item information
    el = self.applyElementSettings(etree.Element('div'), settings)
    el.set('id', '#child-objects')
    el.set('class', 'section scrollspy')
    self._listhelper(node, 'Child Objects', el, self._child_objects)
    return el

  def _listhelper(self, node, title, parent, items):
    """
    Helper method for dumping link lists.

    Args:
      node[dict]: YAML data node.
      title[str]: The level two header to apply to lists.
      parent[etree.Element]: The parent element the headers and lists are to be applied
      items[dict]: Dictionary of databases containing link information
    """

    has_items = False
    for k, db in items.iteritems():
      if self._name in db:
        has_items = True
        h3 = etree.SubElement(parent, 'h3')
        h3.text = k
        ul = etree.SubElement(parent, 'ul')
        ul.set('style', "max-height:350px;overflow-y:Scroll")
        for j in db[self._name]:
          ul.append(j.html())

    if has_items:
      h2 = etree.Element('h2')
      h2.text = title
      parent.insert(0, h2)
