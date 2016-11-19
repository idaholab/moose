import os
import copy
import bs4
import jinja2
import re
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
  def __init__(self, *args, **kwargs):
    super(MoosePage, self).__init__(*args, **kwargs)


    self._html = None
    self._content = ''
    self._meta = dict()

    # Determine url
    local, _ = os.path.splitext(os.path.relpath(self.path, os.path.join(os.getcwd(), 'content')))
    if os.path.basename(local) == 'index':
      local = os.path.dirname(local)
    self._url = os.path.join(local, 'index.html')

    with open(self.path, 'r') as fid:
      content = fid.read().decode('utf-8')

    if self.path.endswith('.md'):
      content, self._meta = self.meta(content)

    self._content = content


  def meta(self, content):

    output = dict()

    count = 0
    lines = content.splitlines()
    for line in lines:

      if line == '':
        break

      match = re.search(r'^(?P<key>[A-Za-z0-9_-]+)\s*:\s*(?P<value>.*)', line)
      if match:
        value = match.group('value')
        if value.lower() == 'false':
          value = False
        elif value.lower() == 'true':
          value = True
        elif value.isnumeric():
          value = float(value)

        output[match.group('key')] = value
        count += 1
      else:
        break


    self.name = output.pop('name', self.name)
    return '\n'.join(lines[count:]), output

  def build(self):
    """
    Converts the markdown to html.
    """

    # Parse the HTML
    self._html = self._parser.convert(self._content)

    template_args = copy.copy(self._template_args)
    template_args.update(self._meta)

    # Create the template object
    env = jinja2.Environment(loader=jinja2.FileSystemLoader([os.path.join(MooseDocs.MOOSE_DIR, 'docs', 'templates'),
                                                             os.path.join(os.getcwd(), 'templates')]))
    template = env.get_template(self._template)

    # Render the html via template
    complete = template.render(current=self, navigation=self._navigation, **template_args)

    # Make sure the destination directory exists
    destination = os.path.join(self.site_dir, self.url())
    if not os.path.exists(os.path.dirname(destination)):
      os.makedirs(os.path.dirname(destination))

    # Finalize the html
    soup = self.finalize(bs4.BeautifulSoup(complete, 'html.parser'))

    # Write the file
    with open(destination, 'w') as fid:
      log.debug('Creating {}'.format(destination))
      fid.write(soup.encode('utf-8'))

  def finalize(self, soup):
    """
    Finalize the html:

      1. Converts *.md links to link to the correct html file.
    """

    def finder(node, desired, pages):
      """
      Locate nodes for the 'desired' filename
      """
      if node.path and node.path.endswith(desired):
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
          log.error('Failed to locate page for markdown file {} in {}'.format(href, self.path))
          #link.attrs.pop('href')
          link['class'] = 'moose-bad-link'
          continue

        elif len(found) > 1:
          msg = 'Found multiple pages matching the supplied markdown file {} in {}:'.format(href, self.path)
          for f in found:
            msg += '\n    {}'.format(f.path)
          log.error(msg)

        # Update the link with the located page
        url = self.relpath(found[0].url())
        log.debug('Converting link: {} --> {}'.format(href, url))
        link['href'] = url

    # Fix <pre><code class="python"> to be <pre class="language-python"><code>
    # Add code copy button
    count = 0
    for pre in soup('pre'):
      code = pre.find('code')
      if code and code.has_attr('class'):
        pre['class'] = 'language-{}'.format(code['class'][0])

        if not code.has_attr('id'):
          code['id'] = 'moose-code-block-{}'.format(str(count))
          count += 1

        id = '#{}'.format(code['id'])
        btn = soup.new_tag('button')
        btn['class'] = "moose-copy-button btn"
        btn['data-clipboard-target'] = id
        btn.string = 'copy'

        if "moose-code-div" in pre.parent['class']:
            pre.parent.insert(0, btn)
        else:
            pre.insert(0, btn)

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

    # Fix media links
    for img in soup('img'):
      img['src'] = self.relpath(img['src'])

    return soup

  def links(self, repo_url):
    """
    Return the GitHub address for editing the markdown file.

    Args:
      repo_url[str]: Web address to use as the base for creating the edit link
    """

    output = [('Edit Markdown', os.path.join(repo_url, 'edit', 'devel', MooseDocs.relpath(self.path)))]

    name = self.breadcrumbs[-1].name

    for key, syntax in self.syntax.iteritems():
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
    print self.path
    soup = bs4.BeautifulSoup(self._html, 'html.parser')
    for tag in soup.find_all(level):
      if 'id' in tag.attrs:
        yield (tag.contents[0], tag.attrs['id'])
     # else:
     #   yield (tag.contents[0], tag.contents[0].lower().replace(' ', '-'))

  def html(self):
    """
    Return the generated html from markdown.
    """
    return self._html
