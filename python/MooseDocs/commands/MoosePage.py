import os
import copy
import bs4
import jinja2
import logging
import MooseDocs
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
  def __init__(self, parser=None, syntax=dict(), filename=None, **kwargs):
    super(MoosePage, self).__init__(**kwargs)

    # Storage for the Markdown parser and the html to be generated
    self._parser = parser
    self._syntax = syntax
    self._html = None

    # Public members
    self.filename = MooseDocs.abspath(filename)

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
    env = jinja2.Environment(loader=jinja2.FileSystemLoader([os.path.join(MooseDocs.MOOSE_DIR, 'docs', 'templates'),
                                                             os.path.join(os.getcwd(), 'templates')]))
    template = env.get_template(self._template)

    # Render the html via template
    config.update(kwargs)
    complete = template.render(current=self, tree=self.root(), **config)

    # Make sure the destination directory exists
    destination = os.path.join(self.site_dir, self.url())
    if not os.path.exists(os.path.dirname(destination)):
      os.makedirs(os.path.dirname(destination))

    # Finalize the html
    soup = self.finalize(bs4.BeautifulSoup(complete, 'html.parser'))

    # Write the file
    with open(destination, 'w') as fid:
      log.debug('Creating {}: using template {}'.format(destination, self._template))
      fid.write(soup.prettify().encode('utf-8'))


  def finalize(self, soup):
    """
    Finalize the html:

      1. Converts *.md links to link to the correct html file.
    """

    def finder(node, desired, pages):
      """
      Locate nodes for the 'desired' filename
      """
      if node.filename and node.filename.endswith(desired):
        pages.append(node)
      for child in node.children:
        finder(child, desired, pages)
      return pages

    # Loop over <a> tags and update links containing .md to point to .html
    for link in soup('a'):
      href = link.get('href')
      if href and (not href.startswith('http')) and href.endswith('.md'):
        found = []
        finder(self.root(), href, found)

        # Error if file not found or if multiple files found
        if not found:
          log.error('Failed to locate page for markdown file {} in {}'.format(href, self.filename))
          continue

        elif len(found) > 1:
          msg = 'Found multiple pages matching the supplied markdown file {} in {}:'.format(href, self.filename)
          for f in found:
            msg += '\n    {}'.format(f.filename)
          log.error(msg)

        # Update the link with the located page
        url = self.relpath(found[0].url())
        log.debug('Converting link: {} --> {}'.format(href, url))
        link['href'] = url

    # Fix <pre><code class="python"> to be <pre class="language-python"><code>
    for pre in soup('pre'):
      code = pre.find('code')
      if code and 'class' in code.attrs:
        pre['class'] = 'language-{}'.format(code['class'])

    # Add materialize sections for table-of-contents
    div = soup.find('div', id='moose-markdown-content')
    if div:
      current = div
      for tag in div.contents:
        if isinstance(tag, bs4.element.Tag):
          if tag.name == 'h2':
            current = soup.new_tag('div', id=tag.get('id', '#'))
            current.attrs['class'] = "section scrollspy"

          if current != tag.parent:
            tag.wrap(current)

    return soup


  def links(self, repo_url):
    """
    Return the GitHub address for editing the markdown file.

    Args:
      repo_url[str]: Web address to use as the base for creating the edit link
    """

    output = [('Edit Markdown', os.path.join(repo_url, 'edit', 'devel', MooseDocs.relpath(self.filename)))]

    name = self._breadcrumbs[-1].name

    for key, syntax in self._syntax.iteritems():
      if syntax.hasObject(name):
        include = syntax.filenames(name)[0]
        rel_include = MooseDocs.relpath(include)
        output.append( ('Header', os.path.join(repo_url, 'blob', 'master', rel_include)) )

        source = include.replace('/include/', '/src/').replace('.h', '.C')
        if os.path.exists(source):
          rel_source = MooseDocs.relpath(source)
          output.append( ('Source', os.path.join(repo_url, 'blob', 'master', rel_source)) )

        output.append( ('Doxygen', syntax.doxygen(name)) )

    return output


  def contents(self, level='h2'):
    """
    Return the table of contents.
    """
    soup = bs4.BeautifulSoup(self._html, 'html.parser')
    for tag in soup.find_all(level):
      if 'id' in tag.attrs:
        yield (tag.contents[0], tag.attrs['id'])
      else:
        yield (tag.contents[0], tag.contents[0].lower().replace(' ', '-'))



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
