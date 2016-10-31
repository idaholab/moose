import os
import copy
import bs4
import jinja2
import logging
log = logging.getLogger(__name__)

from NavigationNode import NavigationNode


class MoosePage(NavigationNode):
  """
  Navigation item for markdown page.

  Args:
    filename[str]: The complete markdown/html filename to be converted.
    parser[markdown.Markdown(): Python Markdown object instance.
    root[str]: The root directory.


  NOTE: This class can also handle pure html pages as well, the markdown
        conversion step is simply skipped if the file ends with .html.
  """

  def __init__(self, filename=None, parser=None, **kwargs):
    super(MoosePage, self).__init__(**kwargs)

    # Public members
    self.filename = filename

    # Storage for the Markdown parser and the html to be generated
    self._parser = parser
    self._html = None

    # Populate the list of parent nodes (i.e., "breadcrumbs")
    self._breadcrumbs = []
    def helper(node):
        if node.parent:
            self._breadcrumbs.insert(0, node)
            helper(node.parent)
    helper(self)

    # Set the URL for the page
    local = [node.name for node in self._breadcrumbs] + ['index.html']
    self._url = os.path.join(*local).lower().replace(' ', '_')

  def _string(self, level):
    """
    Overrides default to include the markdown file name in the tree dump.
    """
    return "{}{}: {}\n".format(' '*2*level, self.name, self.filename)

  def build(self, **kwargs):
    """
    Converts the markdown to html.
    """

    # Create a local configuration
    config = copy.copy(self._config)
    config.update(kwargs)

    # Parse the HTML
    with open(self.filename, 'r') as fid:
      content = fid.read().decode('utf-8')
    if self.filename.endswith('.md'):
        self._html = self._parser.convert(content)
    else:
        self._html = content

    # Create the template object
    env = jinja2.Environment(loader=jinja2.FileSystemLoader('templates'))
    template = env.get_template(self._template)

    # Render the html via template
    config.update(kwargs)
    complete = template.render(current=self, tree=self.root(), **config)

    # Make sure the destination directory exists
    destination = os.path.join(self.site_dir, self.url())
    if not os.path.exists(os.path.dirname(destination)):
      os.makedirs(os.path.dirname(destination))

    # Write the file
    with open(destination, 'w') as fid:
      log.info('Creating {}: {}'.format(destination, self._template))
      soup = bs4.BeautifulSoup(complete, 'html.parser')
      fid.write(soup.prettify().encode('utf-8'))


  def relpath(self, input):
    """
    Returns the relative path to the supplied path compared to the current page.

    Args:
      input[tuple]: The os.path.relpath arguments.
    """
    if input.startswith('http'):
      return input
    return os.path.relpath(os.path.join(self.site_dir, input), os.path.join(self.site_dir, os.path.dirname(self.url())))


  def edit(self, repo_url):
    """
    Return the GitHub address for editing the markdown file.

    Args:
      repo_url[str]: Web address to use as the base for creating the edit link
    """
    return os.path.join(repo_url, 'edit', 'devel', 'docs', self.filename)


  def contents(self, level='h2'):
    """
    Return the table of contents.
    """
    soup = bs4.BeautifulSoup(self._html, 'html.parser')
    for tag in soup.find_all(level):
      yield (tag.contents[0], tag.attrs['id'])


  def breadcrumbs(self):
    """
    Return the parent nodes (i.e., "breadcrumbs")
    """
    return self._breadcrumbs


  def url(self):
    """
    Return the url to the generated page.
    """
    return self._url


  def html(self):
    """
    Return the generated html from markdown.
    """
    return self._html
