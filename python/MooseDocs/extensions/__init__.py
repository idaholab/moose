from MooseMarkdown import MooseMarkdown
from markdown.util import etree


def full_name(node, action, group_name='.'):
  """
  Return the full markdown file name of a node for MOOSE system or object.

  Inputs:
    node: YAML node of interest.
    action[str]: 'object' or 'system' specifying what type of name desired
    group_name[str]: The name of the documentation group (e.g., framework or phase_field) an object belongs
  """
  name = node['name']
  full_name = name.strip('/').split('/')
  if '<type>' in full_name:
    full_name.remove('<type>')
  if '*' in full_name:
    full_name.remove('*')

  if action == 'object':
    full_name.insert(-1, group_name)
    full_name[-1] += '.md'
  elif action == 'system':
    full_name.insert(0, 'systems')
    full_name.append('index.md')

  return '/'.join(full_name)


def system_name(node):
  """
  Returns markdown filename for a system.
  """
  return full_name(node, 'system')


def object_name(node, group_name):
  """
  Returns markdown filename for an object.
  """
  return full_name(node, 'object', group_name)


def get_collection_items(node, group_name, group_syntax, action):
  """
  Returns li html elements for each object in a system.

  Input:
    node: YAML node of interest.
    group_name[str]: The name of the documentation group (e.g., framework or phase_field) an object belongs
    group_syntax: MooseApplicationSyntax object.
    action[str]: 'object' or 'system' specifying what type of name desired
  """
  items = []

  for child in node['subblocks']:

    name = child['name']
    if name.endswith('*') or group_syntax.hidden(name):
      continue
    short_name = name.split('/')[-1].strip()

    if action == 'object':
        has_items = group_syntax.hasObject(short_name)
    elif action == 'system':
        has_items = group_syntax.hasSystem(name)
    else:
        raise Exception('Unknown value for "action" variable ({}) supplied.'.format(action))

    if has_items:
      item = etree.Element('li')
      items.append(item)

      header = etree.SubElement(item, 'div')
      header.set('class', 'collapsible-header')

      header_row = etree.SubElement(header, 'div')
      header_row.set('class', 'row')

      item_name= etree.SubElement(header_row, 'div' )
      item_name.set('class', 'moose-collection-name col l4')

      a = etree.SubElement(item_name, 'a')
      a.text = short_name
      a.set('href', full_name(child, action, group_name))

      description = child['description'].strip()
      if description:
        item_desc = etree.SubElement(header_row, 'div')
        item_desc.set('class', 'moose-collection-description hide-on-med-and-down col l8')
        item_desc.text = description

        body = etree.SubElement(item, 'div')
        body.set('class', 'collapsible-body')
        body.text = description

  return items


def create_collection(node, syntax, action, groups=[]):
  """
  Creates subobject collection for a given node from the YAML dump.

  Inputs:
    node: The YAML node to inspect.
    syntax: The dict of MooseApplicationSyntax objects
    action[str]: Name of test function on the MooseApplicationSyntax object to call ('system' or 'object')
    groups[list]: The list of groups to restrict the collection.
  """

  groups = groups if groups else syntax.keys()
  use_header = True if len(groups) > 1 else False

  children = []
  for name in groups:
    items = get_collection_items(node, name, syntax[name], action)
    if items and use_header:
      li = etree.Element('li')
      header = etree.SubElement(li, 'div')
      header.set('class', 'collapsible-header moose-group-header')
      header.text = '{} {}'.format(syntax[name].name(), '{}s'.format(action.title()))
      items.insert(0, header)
    children += items

  collection = None
  if children:
    collection = etree.Element('ul')
    collection.set('class', 'collapsible')
    collection.set('data-collapsible', 'accordion')
    collection.extend(children)

  return collection


def create_object_collection(node, syntax, **kwargs):
  """
  Return html element listing objects.
  """
  return create_collection(node, syntax, 'object', **kwargs)

def create_system_collection(node, syntax, **kwargs):
  """
  Return html element listing systems.
  """
  return create_collection(node, syntax, 'system', **kwargs)
