import os

class NavigationNode(object):
  """
  Class for building navigation tree for website.

  Args:
    name[str]: The name of the node.
    parent[NavigationNode]: The parent node in tree.
  """
  def __init__(self, name='', parent=None, site_dir='', template=None, filename=None, **kwargs):

    # Public member variables, these are accessed by the Jinja2 template.
    self.filename = filename
    self.name = name
    self.parent = parent
    self.children = []
    self.site_dir = site_dir

    self._config = kwargs
    self._template = template
    self._pages = [] # A flat list of NavigationNode objects (see initialize)

  def __eq__(self, other):
    """
    Tests if this object is equivalent to the supplied object.
    """
    return isinstance(other, self.__class__) and self.name == other.name and self.parent == other.parent and self.children == other.children

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


  def build(self, **kwargs):
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
    for child in self.children:
      if child.name == 'Overview':
        return child.url()
    return None

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
    output = "{}{}\n".format(' '*2*level, self.name)
    for child in self.children:
      output += child._string(level=level+1)
    return output
