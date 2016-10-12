import re
import os
import logging
log = logging.getLogger(__name__)

from markdown.util import etree
from MooseSyntaxBase import MooseSyntaxBase
import utils
import MooseDocs

class MooseSystemSyntax(MooseSyntaxBase):
  """
  Creates tables of sub-object/systems.

  !subobjects = A list of subobjects /* and /<type>
  !subsystems = A list of sub-syntax (e.g, Markers and Indicators)
  """

  RE = r'^!(subobjects|subsystems)\s+(.*?)\s+(.*?)(?:$|\s+)(.*)'

  def __init__(self, yaml=None, syntax=None, **kwargs):
    MooseSyntaxBase.__init__(self, self.RE, yaml=yaml, syntax=syntax, **kwargs)

  def handleMatch(self, match):
    """
    Handle the regex match.
    """

    # Extract match options and settings
    action = match.group(2)
    group = match.group(3)
    syntax = match.group(4)
    settings, styles = self.getSettings(match.group(5))

    # Error if supplied group is invalid
    if group not in self._syntax:
      return self.createErrorElement(message="The group name '{}' was not found.".format(group))

    if action == 'subobjects':
      el = self.subobjectsElement(group, syntax, styles)
    elif action == 'subsystems':
      el = self.subsystemsElement(group, syntax, styles)
    return el

  def subobjectsElement(self, group, syntax, styles):
    """
    Create table of sub-objects.
    """

    node = self._yaml.find(os.path.join(syntax, '*'))
    if node:
      node = self._yaml.find(syntax)
    elif not node:
      node = self._yaml.find(os.path.join(syntax, '<type>'))

    if not node:
      return self.createErrorElement("The are not any sub-objects for the supplied syntax: {}".format(syntax))

    table = MooseDocs.MarkdownTable('Name', 'Description')
    for child in node['subblocks']:
      name = child['name']
      if name.endswith('*'):
        continue

      name = name.split('/')[-1].strip()
      if self._syntax[group].hasObject(name):
        a = etree.Element('a')
        a.set('href', '{}/index.html'.format(name))
        a.text = name
        table.addRow(a, child['description'])

    if table.size() == 0:
      return self.createErrorElement("No sub-objects exists for the supplied syntax: {}".format(syntax))

    el = etree.Element('div', styles)
    h2 = etree.SubElement(el, 'h2')
    h2.text = 'Available Sub-Objects'
    el.append(table.html())
    return el

  def subsystemsElement(self, group, syntax, styles):
    """
    Create table of sub-systems.
    """

    node = self._yaml.find(syntax)
    if not node:
      return createErrorElement("The are not any sub-systems for the supplied syntax: {} You likely need to remove the '!subobjects' syntax.".format(syntax))

    table = MooseDocs.MarkdownTable('Name', 'Description')
    if node['subblocks']:
      for child in node['subblocks']:
        name = child['name']
        if self._syntax[group].hasSystem(name):
          name = name.split('/')[-1].strip()
          a = etree.Element('a')
          a.set('href', '{}/Overview'.format(name))
          a.text = name
          table.addRow(a, child['description'])

    if table.size() == 0:
      return self.createErrorElement("No sub-systems exists for the supplied syntax: {}. You likely need to remove the '!subsystems' markdown.".format(syntax))

    el = etree.Element('div', styles)
    h2 = etree.SubElement(el, 'h2')
    h2.text = 'Available Sub-Systems'
    el.append(table.html())
    return table.html()
