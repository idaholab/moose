import os
import copy
import bs4
import jinja2
import logging
log = logging.getLogger(__name__)

import MooseDocs
from MooseDocsNode import MooseDocsNode

class MooseDocsMarkdownNodeBase(MooseDocsNode):
    """
    Node for converting markdown to html.
    """
    def __init__(self, md_file=None, parser=None, template=None, template_args=dict(), **kwargs):
        super(MooseDocsMarkdownNodeBase, self).__init__(**kwargs)

        if (not md_file) or (not os.path.exists(md_file)):
            raise Exception('The supplied markdown file must exists: {}'.format(md_file))

        # Extract the MooseLinkDatabase for creating source and doxygen links
        ext = MooseDocs.get_moose_markdown_extension(parser)
        self._syntax = ext.syntax if ext else dict()

        self._parser = parser
        self._template = template
        self._template_args = template_args
        self._html = None
        self._md_file = md_file

    def source(self):
        """
        Return the source markdown file.
        """
        return self._md_file

    def environment(self):
        """
        Return the jinja2 template environment.

        This is a method so that child objects can modify the environment as needed (e.g., add global scripts).
        """
        env = jinja2.Environment(loader=jinja2.FileSystemLoader([os.path.join(MooseDocs.MOOSE_DIR, 'docs', 'templates'),
                                                                 os.path.join(os.getcwd(), 'templates')]))
        return env

    def build(self, lock=None):
        """
        Converts the markdown to html.
        """

        # Read the markdown and parse the HTML
        log.debug('Parsing markdown: {}'.format(self._md_file))
        content, meta = MooseDocs.read_markdown(self._md_file)
        self._html = self._parser.convert(content)

        template_args = copy.copy(self._template_args)
        template_args.update(meta)

        # Create the template object
        env = self.environment()
        template = env.get_template(self._template)

        # Render the html via template
        complete = template.render(current=self, **template_args)

        # Make sure the destination directory exists, if it already does do nothing. If it does not exist try to create
        # it, but include a try statement because it might get created by another process.
        destination = self.path()
        if lock: # Lock is not supplied or needed with build function is called from the liveserver
            with lock:
                if not os.path.exists(destination):
                    os.makedirs(destination)
        else:
            if not os.path.exists(destination):
                os.makedirs(destination)

        # Finalize the html
        soup = self.finalize(bs4.BeautifulSoup(complete, 'html.parser'))

        # Write the file
        with open(self.url(), 'w') as fid:
            log.debug('Creating {}'.format(self.url()))
            fid.write(soup.encode('utf-8'))

    def finalize(self, soup):
        """
        Finalize the html prior to output.
        """

        # Fix media links
        for img in soup('img'):
            img['src'] = self.relpath(img['src'])

        # Fix <pre><code class="python"> to be <pre class="language-python"><code> and add a copy button.
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

        return soup

    def content(self):
        """
        Return the generated html from markdown.
        """
        if self._html is None:
            raise Exception('The "build" command must be executed prior to extracting content.')
        return self._html

    def url(self, parent=None):
        """
        Return the url to the page to be created.
        """
        path = self.path()
        if parent:
            path = os.path.relpath(parent.path(), path)
        return os.path.join(path, 'index.html')
