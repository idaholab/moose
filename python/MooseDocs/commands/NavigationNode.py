import os
import MooseDocs

class NavigationNode(object):
  """
  Class for building navigation tree for website.

  Args:
    name[str]: The name of the node.
    parent[NavigationNode]: The parent node in tree.
  """
  def __init__(self, path='.', parent=None, site_dir=None, syntax=None, name=None,
               parser=None, navigation=None, template=None, template_args=None):

    # Public member variables, these are accessed by the Jinja2 template.
    self.path = path
    self.parent = parent
    self.site_dir = site_dir
    self.syntax = syntax
    self._parser = parser
    self._navigation = navigation
    self._template = template
    self._template_args = template_args
    self._url = None

    if name == None:
      if self.path.endswith('index.html') or self.path.endswith('index.md'):
        name = os.path.basename(os.path.dirname(self.path))
      else:
        name, _ = os.path.splitext(os.path.basename(self.path))
      name = ' '.join([n[0].upper() + n[1:] for n in name.split('_')]) # Don't use title() because that will lower case within the string

    self.name = name

    self.children = []

    self.breadcrumbs = []
    def helper(node):
      self.breadcrumbs.insert(0, node)
      if node.parent:
        helper(node.parent)
    helper(self)


  def __eq__(self, other):
    """
    Tests if this object is equivalent to the supplied object.
    """
    return isinstance(other, self.__class__) and self.path == other.path and self.parent == other.parent and self.children == other.children

  def __str__(self):
    """
    Allows 'print' to dump the complete tree structure.
    """
    return self._string()


  def root(self):
    """
    Returns the "root" node of this node.
    """

    def helper(node):
      if node.parent:
        return helper(node.parent)
      else:
        return node
    return helper(self)


  def relpath(self, input):
    """
    Returns the relative path to the supplied path compared to the current page.

    Args:
      input[tuple]: The os.path.relpath arguments.
    """
    if input.startswith('http'):
      return input

    return os.path.relpath(os.path.join(self.site_dir, input), os.path.join(self.site_dir, os.path.dirname(self.url())))

  def mediapath(self, input):
    """
    Returns relative path for 'media' files.
    """
    local = os.path.relpath(input, os.path.join(os.getcwd(), 'docs'))
    return local

  def build(self, *args, **kwargs):
    """
    Method for constructing state (e.g., converting markdown).

    NOTE: This is called in parallel so accessing parent/child data
          should be avoided.
    """
    pass

  def url(self):
    """
    Return the url() for the node.
    """
    return self._url

  def active(self, page):
    """
    Tests if the supplied node is contained in the current tree.
    """

    def helper(tree):
      for child in tree.children:
        for h in helper(child):
          yield h
        else:
          yield page == child

    return 'active' if (page == self or any(helper(self))) else ''


  def _string(self, level=0):
    """
    Helper function for dumping the tree.
    """
    output = "{}{}: {} {}\n".format(' '*2*level, self.name, self.path, type(self))
    for child in self.children:
      output += child._string(level=level+1)
    return output
