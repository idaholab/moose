import os
import markdown
import logging
log = logging.getLogger(__name__)

import MooseDocs
from MooseDocs.commands.MooseDocsMarkdownNodeBase import MooseDocsMarkdownNodeBase

class MooseMarkdown(markdown.Markdown):
    """
    A custom Markdown object for handling raw text, markdown files, or MooseDocsMarkdownNode objects.

    Additionally, this class automatically loads the required markdown extensions.

    The key to this class is allowing the Markdown object to work with MooseDocsMarkdownNode objects such that the
    extension objects, namely MooseTemplate, could have access to the node object to allow for searching the tree
    for other pages. This should allow for cross page figure, equation, and table links to be created.
    """

    def __init__(self, extensions=[], extension_configs=dict(), **kwargs):

        # The member for holding the current MooseDocsMarkdownNodeBase object
        self.current = None

        # Add the required packages
        extensions += ['toc', 'smarty', 'admonition', 'extra', 'meta', 'mdx_math', 'markdown_include.include']

        # Configure packages
        for config in ['mdx_math', 'markdown_include.include']:
            if config not in extension_configs:
                extension_configs[config] = dict()
                extension_configs['mdx_math'].setdefault('enable_dollar_delimiter', True)
        extension_configs['markdown_include.include'].setdefault('base_path', MooseDocs.ROOT_DIR)
        super(MooseMarkdown, self).__init__(extensions=extensions, extension_configs=extension_configs, **kwargs)

    def convert(self, node):
        """
        Convert the raw text, markdown file, or node to html.

        Args:
            content[str]: A markdown file or markdown content.
        """
        if isinstance(node, MooseDocsMarkdownNodeBase):
            with open(node.source(), 'r') as fid:
                md = fid.read().decode('utf-8')
            self.current = node
            log.debug('Parsing markdown: {}'.format(node.source()))
            html = super(MooseMarkdown, self).convert(md)
            self.current = None
            return html
        elif os.path.isfile(node):
            with open(node, 'r') as fid:
                md = fid.read().decode('utf-8')
            return super(MooseMarkdown, self).convert(md)
        else:
            return super(MooseMarkdown, self).convert(node)
