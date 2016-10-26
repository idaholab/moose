import re
import os
import copy
import collections
import logging
log = logging.getLogger(__name__)

from markdown.inlinepatterns import Pattern
from markdown.util import etree
from MooseSyntaxBase import MooseSyntaxBase
import utils
import MooseDocs

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
    !devel - Returns links to the source code in the repository and doxygen page.

  "devel" Settings:
    title[str]: The developer link box title (default: Developer Links)

  """

  RE = r'^!(description|parameters|inputfiles|childobjects|devel)\s+(.*?)(?:$|\s+)(.*)'

  def __init__(self, input_files=dict(), child_objects=dict(), repo=None, **kwargs):
    super(MooseObjectSyntax, self).__init__(self.RE, **kwargs)


    # Input arguments
    self._input_files = input_files
    self._child_objects = child_objects
    self._repo = repo
    self._name = None

  def handleMatch(self, match):
    """
    Create a <p> tag containing the supplied description from the YAML dump.
    """

    # Extract match options and settings
    action = match.group(2)
    syntax = match.group(3)

    # Default Settings
    if action == 'devel':
      self._settings['title'] = 'Developer Links'

    # Extract Settings
    settings, styles = self.getSettings(match.group(4))

    # Locate description
    node = self._yaml.find(syntax)
    if not node:
      return self.createErrorElement('Failed to locate {} syntax.'.format(syntax))

    # Determine object name
    self._name = node['name'].split('/')[-1]

    if action == 'description':
      el = self.descriptionElement(node, settings, styles)
    elif action == 'parameters':
      el = self.parametersElement(node, settings, styles)
    elif action == 'inputfiles':
      el = self.inputfilesElement(node, settings, styles)
    elif action == 'childobjects':
      el = self.childobjectsElement(node, settings, styles)
    elif action == 'devel':
      el = self.develElement(node, settings, styles)
    elif action == 'subobjects':
      el = self.subobjectsElement(node, settings, styles)
    return el

  def descriptionElement(self, node, settings, styles):
    """
    Return the class description html element.

    Args:
      node[dict]: YAML data node.
      styles[dict]: Styles from markdown.
    """

    if ('description' not in node) or (not node['description']):
      return self.createErrorElement('Failed to locate class description for {} syntax.'.format(node['name']))

    # Create the html element with supplied styles
    el = self.addStyle(etree.Element('p'), **styles)
    el.text = node['description']
    return el

  def parametersElement(self, node, settings, styles):
    """
    Return table(s) of input parameters.

    Args:
      node[dict]: YAML data node.
      styles[dict]: Styles from markdown.
    """

    # Create the tables (generate 'Required' and 'Optional' initially so that they come out in the proper order)
    tables = collections.OrderedDict()
    tables['Required'] = MooseDocs.MooseObjectParameterTable()
    tables['Optional'] = MooseDocs.MooseObjectParameterTable()

    # Loop through the parameters in yaml object
    for param in node['parameters'] or []:
      name = param['group_name']
      if not name and param['required']:
        name = 'Required'
      elif not name and not param['required']:
        name = 'Optional'

      if name not in tables:
        tables[name] = MooseDocs.MooseObjectParameterTable()
      tables[name].addParam(param)

    el = self.addStyle(etree.Element('div'), **styles)
    title = etree.SubElement(el, 'h2')
    title.text = 'Input Parameters'
    for key, table in tables.iteritems():
      subtitle = etree.SubElement(el, 'h3')
      subtitle.text = '{} {}'.format(key, 'Parameters')
      el.append(table.html())
    return el

  def inputfilesElement(self, node, settings, styles):
    """
    Return the links to input files and child objects.

    Args:
      node[dict]: YAML data node.
      styles[dict]: Styles from markdown.
    """
    # Print the item information
    el = self.addStyle(etree.Element('div'), **styles)
    self._listhelper(node, 'Input Files', el, self._input_files)
    return el

  def childobjectsElement(self, node, settings, styles):
    """
    Return the links to input files and child objects.

    Args:
      node[dict]: YAML data node.
      styles[dict]: Styles from markdown.
    """
    # Print the item information
    el = self.addStyle(etree.Element('div'), **styles)
    self._listhelper(node, 'Child Objects', el, self._child_objects)
    return el

  def develElement(self, node, settings, styles):
    """
    Return the developer doxygen, github links.

    Args:
      node[dict]: YAML data node.
      styles[dict]: Styles from markdown.
    """

    if not self._repo:
      el = createErrorElement("Attempting to create source links to repository, but the 'repo' configuration option was not supplied.")

    el = self.addStyle(etree.Element('div'), **styles)
    title = etree.SubElement(el, 'h4')
    title.text = settings['title']
    ul = etree.SubElement(el, 'ul')

    for key, syntax in self._syntax.iteritems():
      if syntax.hasObject(self._name):
        include = syntax.filenames(self._name)[0]
        rel_include = os.path.relpath(include, self._root)

        p = etree.SubElement(ul, 'li')
        a = etree.SubElement(p, 'a')
        a.set('href', os.path.join(self._repo, rel_include))
        a.text = os.path.basename(rel_include)

        source = include.replace('/include/', '/src/').replace('.h', '.C')
        if os.path.exists(source):

          p = etree.SubElement(ul, 'li')

          rel_source = os.path.relpath(source, self._root)
          a = etree.SubElement(p, 'a')
          a.set('href', os.path.join(self._repo, rel_source))
          a.text = os.path.basename(rel_source)

          if syntax.doxygen:
            p = etree.SubElement(ul, 'li')
            p.text = 'Doxygen:&nbsp;'
            a = etree.SubElement(p, 'a')
            a.set('href', "{}class{}.html".format(syntax.doxygen, self._name))
            a.text = self._name

    return el


  def _listhelper(self, node, title, parent, items):
    """
    Helper method for dumping link lists.

    Args:
      node[dict]: YAML data node.
      title[str]: The level two header to apply to lists.
      parent[etree.Element]: The parent element the headers and lists are to be applied
      items[dict]: Dictionary of databases containg link information
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
