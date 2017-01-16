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

  Available Settings:
    title[str]: Set the title of the section defined above the table.
  """

  RE = r'^!(subobjects|subsystems)\s+(.*?)(?:$|\s+)(.*)'

  def __init__(self, yaml=None, syntax=None, **kwargs):
    MooseSyntaxBase.__init__(self, self.RE, yaml=yaml, syntax=syntax, **kwargs)
    self._settings['title'] = None

  def handleMatch(self, match):
    """
    Handle the regex match.
    """

    # Extract match options and settings
    action = match.group(2)
    syntax = match.group(3)
    settings = self.getSettings(match.group(4))

    if action == 'subobjects':
      el = self.subobjectsElement(syntax, settings)
    elif action == 'subsystems':
      el = self.subsystemsElement(syntax, settings)
    return el

  def subobjectsElement(self, obj_name, settings):
    """
    Create table of sub-objects.
    """

    node = self._yaml.find(os.path.join(obj_name, '*'))
    if node:
      node = self._yaml.find(obj_name)
    elif not node:
      node = self._yaml.find(os.path.join(obj_name, '<type>'))

    if not node:
      return self.createErrorElement("The are not any sub-objects for the supplied syntax: {}".format(obj_name))

    el = self.applyElementSettings(etree.Element('div'), settings)
    h2 = etree.SubElement(el, 'h2')
    h2.text = settings['title'] if settings['title'] else 'Available Sub-Objects'
    collection = MooseDocs.extensions.create_object_collection(node, self._syntax)
    if collection:
      el.append(collection)
    return el

  def subsystemsElement(self, sys_name, settings):
    """
    Create table of sub-systems.
    """

    node = self._yaml.find(sys_name)
    if not node:
      return createErrorElement("The are not any sub-systems for the supplied syntax: {} You likely need to remove the '!subobjects' syntax.".format(sys_name))

    el = self.applyElementSettings(etree.Element('div'), settings)
    h2 = etree.SubElement(el, 'h2')
    h2.text = settings['title'] if settings['title'] else 'Available Sub-Systems'
    collection = MooseDocs.extensions.create_system_collection(node, self._syntax)
    if collection:
      el.append(collection)

    return el
