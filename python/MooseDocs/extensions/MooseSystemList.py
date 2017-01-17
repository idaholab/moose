import re
import os
import logging
log = logging.getLogger(__name__)

from markdown.util import etree
from MooseSyntaxBase import MooseSyntaxBase
import utils
import MooseDocs

class MooseSystemList(MooseSyntaxBase):
  """
  Creates dynamic lists of sub-object/systems.

  Examples:
  !systems
  !systems framework phase_field
  """

  RE = r'^!systems\s*(.*)'

  def __init__(self, yaml=None, syntax=None, **kwargs):
    MooseSyntaxBase.__init__(self, self.RE, yaml=yaml, syntax=syntax, **kwargs)
    self._settings['groups'] = None

  def handleMatch(self, match):
    """
    Handle the regex match for this extension.
    """

    def group_name(groups, name):
      for group in groups:
        if self._syntax[group].hasSystem(name):
          return self._syntax[group].name()
      return None

    def hidden(groups, name):
      return all([obj.hidden(name) for obj in self._syntax.itervalues()])

    # Extract settings
    settings = self.getSettings(match.group(2))

    # Extract the data to consider
    groups = self._syntax.keys()
    if settings['groups']:
      groups = settings['groups'].split()
    data = self._yaml.get()

    # The primary element
    el = etree.Element('div')
    el.set('class', 'moose-system-list')
    def add_li(items, parent, level=0):
      """
      Helper for building nested lists of objects.
      """
      for item in items:

        name = item['name']
        short_name = name.split('/')[-1].strip()

        if short_name == '<type>' or hidden(groups, name):
          continue

        id = short_name.replace(' ', '-').lower()

        div = etree.SubElement(parent, 'div')
        if level == 0:
          div.set('class', 'section scrollspy')
          div.set('id', id)

        h = etree.SubElement(div, 'h{}'.format(str(level+2)))
        h.text = short_name
        h.set('id', id)

        a = etree.SubElement(h, 'a')
        a.set('href', MooseDocs.extensions.system_name(item))
        i = etree.SubElement(a, 'i')
        i.set('class', 'material-icons')
        i.text = 'input'

        gname = group_name(groups, name)
        if gname:
          tag = etree.SubElement(h, 'div')
          tag.set('class', 'chip moose-chip')
          tag.text = gname

        if name.endswith('<type>'):
          collection = MooseDocs.extensions.create_object_collection(item, self._syntax, groups=groups)
          if collection:
            div.append(collection)

        elif item['subblocks']:
          if any([child['name'].endswith('*') for child in item['subblocks']]):
            collection = MooseDocs.extensions.create_object_collection(item, self._syntax, groups=groups)
            if collection:
              div.append(collection)
          else:
            add_li(item['subblocks'], etree.SubElement(div, 'div'), level+1)

    add_li(data, el)

    # Remove headings that don't contain objects
    # TODO: This hides actions that are not supposed to have objects, but for now we just hide
    for tag in list(el):
      has_collection = False
      for item in tag.iter('ul'):
        if ('class' in item.attrib):
          has_collection = True
      if not has_collection:
        el.remove(tag)
    return el
