#pylint: disable=missing-docstring
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html
#pylint: enable=missing-docstring

import os
import copy
import logging

import jinja2
import bs4
from markdown.postprocessors import Postprocessor

import mooseutils

import MooseDocs
from MooseMarkdownExtension import MooseMarkdownExtension
from app_syntax import AppSyntaxExtension

from .. import common

LOG = logging.getLogger(__name__)

class TemplateExtension(MooseMarkdownExtension):
    """
    Extension for applying template to converted markdown.
    """
    @staticmethod
    def defaultConfig():
        """TemplateExtension configuration."""
        config = MooseMarkdownExtension.defaultConfig()
        config['template'] = ['', "The jinja2 template to apply."]
        config['template_args'] = [dict(), "Arguments passed to the MooseTemplate Postprocessor."]
        config['environment_args'] = [dict(), "Arguments passed to the jinja2.Environment."]
        config['doxygen'] = ['', "Provide a URL to append with Doxygen link to class."]
        config['repo'] = ['', "The remote repository to create hyperlinks "
                              "(e.g., http://github.com/idaholab/moose)."]
        config['branch'] = ['master', "The branch name to consider in repository links."]
        return config

    def extendMarkdown(self, md, md_globals):
        """
        Applies template to html converted from markdown.
        """
        md.registerExtension(self)
        config = self.getConfigs()

        md.postprocessors.add('moose_template',
                              TemplatePostprocessor(markdown_instance=md, **config), '_end')

def makeExtension(*args, **kwargs): #pylint: disable=invalid-name
    """Create TemplateExtension"""
    return TemplateExtension(*args, **kwargs)

class TemplatePostprocessorBase(Postprocessor):
    """
    Base for creating extensions that apply markdown converted html content to an jinja2 template.

    Generally, to create template extension there are only two methods that should be overridden:
        globals: Allows for new functions to be added to the jinja2.Environment.
        arguments: Allows for new template arguments to be added prior to application.

    For an example, see the TemplatePostprocessor in this file.

    NOTE: Be sure to call the base class of these methods to get the functionality of this base
          class. However, both expect the inputs to be modified in-place (i.e., pass by reference),
          thus no return statements are needed.
    """
    def __init__(self, markdown_instance, **config):
        super(TemplatePostprocessorBase, self).__init__(markdown_instance)
        self._template = config.pop('template')
        self._template_args = config.pop('template_args', dict())
        self._environment_args = config.pop('environment_args', dict())

        # Storage for node property.
        self._node = None

        # The 'markdown.extensions.meta' extension is required, but the 'meta' extension doesn't get
        # registered so the list of preprocessors is checked.
        try:
            self.markdown.preprocessors.index('meta')
        except ValueError:
            raise mooseutils.MooseException("The 'meta' extension is required.")

    @property
    def node(self):
        """
        Return the current MooseDocsNode object.
        """
        self._node = self.markdown.current
        return self._node

    def globals(self, env):
        """
        Defines global template functions. (virtual)

        Args:
            env[jinja2.Environment]: Template object for adding global functions.
        """
        env.globals['insert_files'] = self._insertFiles
        env.globals['relpath'] = self._relpath
        env.globals['breadcrumbs'] = self._breadcrumbs
        env.globals['load'] = self._load
        env.globals['display'] = self._display

    def arguments(self, template_args, text): #pylint: disable=no-self-use
        """
        Method for modifying the template arguments to be applied to the jinja2 templates engine.

        Args:
            template_args[dict]: Template arguments to be applied to jinja2 template.
            text[str]: Convert markdown to be applied via the 'content' template argument.
        """
        template_args.setdefault('content', text)
        template_args.setdefault('navigation', [])
        template_args['page_status'] = self._pageStatus()
        template_args['page_title'] = self._display(self.markdown.current)

    def run(self, text):
        """
        Apply the converted text to an jinja2 template and return the result.

        Args:
            text[str]: Converted markdown to be applied to the template.
        """
        # Update the meta data to proper python types
        meta = dict()
        for key, value in self.markdown.Meta.iteritems():
            meta[key] = eval(''.join(value))

        # Define template arguments
        template_args = copy.copy(self._template_args)
        template_args.update(meta)
        self.arguments(template_args, text)

        # Execute template and return result
        paths = [os.path.join(MooseDocs.MOOSE_DIR, 'docs', 'templates'),
                 os.path.join(os.getcwd(), 'templates')]
        env = jinja2.Environment(loader=jinja2.FileSystemLoader(paths), **self._environment_args)
        self.globals(env)
        template = env.get_template(self._template)
        complete = template.render(current=self.markdown.current, **template_args)

        # Finalize the contents for output
        soup = bs4.BeautifulSoup(complete, 'html.parser')
        self._imageLinks(self.markdown.current, soup)
        self._markdownLinks(self.markdown.current, soup)
        return unicode(soup)

    @staticmethod
    def _insertFiles(filenames):
        """
        Helper function for jinja2 to read css file and return as string.
        """
        if isinstance(filenames, str):
            filenames = [filenames]

        out = []
        for filename in filenames:
            with open(os.path.join(MooseDocs.ROOT_DIR, filename), 'r') as fid:
                out += [fid.read().strip('\n')]
        return '\n'.join(out)

    @staticmethod
    def _imageLinks(node, soup):
        """
        Makes images links relative
        """
        for img in soup('img'):
            if 'src' in img.attrs:
                img['src'] = TemplatePostprocessorBase._relpath(img['src'], node.destination)

    def _markdownLinks(self, node, soup):
        """
        Performs auto linking of markdown files.
        """
        # Loop over <a> tags and update links containing .md to point to .html
        for link in soup('a'):
            href = link.get('href')
            if href and (not href.startswith('http')) \
                    and href.endswith(MooseDocs.common.EXTENSIONS):

                # Split filename from section link (#)
                parts = href.split('#')

                # Populate the list of found files
                filename, found = self.markdown.getFilename(parts[0], check_local=False)

                # Error if file not found or if multiple files found
                if not filename:
                    if link.get('data-moose-disable-link-error', None) is None:
                        if isinstance(node, common.nodes.MarkdownFileNodeBase):
                            LOG.error('Failed to locate file %s in %s',
                                      href, node.filename)
                        else:
                            LOG.error('Failed to locate file %s.', href)
                    link['class'] = 'moose-bad-link'
                    continue

                # Update the link with the located page
                url = os.path.relpath(found.destination, os.path.dirname(node.destination))
                if len(parts) == 2:
                    url += '#' + parts[1]
                LOG.debug('Converting link: %s --> %s', href, url)
                link['href'] = url

    def _pageStatus(self):
        """
        Report the page status for insert into meta data
        """
        return self.markdown.current.status

    @staticmethod
    def _relpath(path, start):
        if path.startswith('http'):
            return path
        return os.path.relpath(path, os.path.dirname(start))

    @staticmethod
    def _breadcrumbs(current):
        """
        Return a list of all the parent nodes.
        """
        crumbs = []
        def breadcrumb_helper(node):
            """Function for building list of parent nodes."""
            crumbs.insert(0, node)
            if node.parent:
                breadcrumb_helper(node.parent)
        breadcrumb_helper(current)
        return crumbs

    def _load(self, location):
        """
        Loads css/js from given url and sets the correct relative path.
        """
        return self._relpath(location, self.markdown.current.destination)

    @staticmethod
    def _display(node):
        """
        Create the breadcumb display name (i.e., separate camel case).
        """
        return mooseutils.camel_to_space(node.display)

class TemplatePostprocessor(TemplatePostprocessorBase):
    """
    A template extension that works with the 'MooseDocs.extensions.app_syntax' extension.
    """
    def __init__(self, *args, **kwargs):
        self._doxygen_url = kwargs.pop('doxygen')

        super(TemplatePostprocessor, self).__init__(*args, **kwargs)

        # The 'MooseDocs.extensions.app_syntax' are required
        self.markdown.requireExtension(AppSyntaxExtension)
        ext = self.markdown.getExtension(AppSyntaxExtension)
        self._repo = ext.getConfig('repo')
        self._database = common.MooseClassDatabase(os.path.join(self._repo,
                                                                'blob',
                                                                ext.getConfig('branch')))
        self._syntax = ext.getMooseAppSyntax()

    def globals(self, env):
        """
        Add MOOSE syntax related functions for the template.
        """
        super(TemplatePostprocessor, self).globals(env)

    def arguments(self, template_args, text):
        """
        Add MOOSE syntax related arguments to the template arguments.
        """
        super(TemplatePostprocessor, self).arguments(template_args, text)
        template_args.setdefault('tableofcontents', self._tableofcontents(text))
        template_args.setdefault('doxygen', self._doxygen())
        template_args.setdefault('github_edit', True)
        template_args.setdefault('code', self._code())
        template_args.setdefault('repo_url', self._repo)
        template_args.setdefault('edit_markdown', self._editMarkdown())

    @staticmethod
    def _tableofcontents(text, level='h2'):
        """
        Returns the h2 (default) tags for the supplied html text.
        """
        soup = bs4.BeautifulSoup(text, 'html.parser')
        for tag in soup.find_all(level):
            if 'id' in tag.attrs and tag.contents:
                yield (tag.contents[0], tag.attrs['id'])

    def _editMarkdown(self):
        """
        Return the url to the markdown file for this object.
        """
        if isinstance(self.node, common.nodes.MarkdownFileNodeBase):
            return os.path.join(self._repo, 'edit', 'devel',
                                os.path.relpath(self.node.filename, self.node.root_directory))
        return None

    def _doxygen(self):
        """
        Return the doxygen link, if it exists.
        """
        if self._doxygen_url:
            nodes = self._syntax.findall('/' + self.node.name)
            if nodes and isinstance(nodes[0], common.nodes.ObjectNode):
                return os.path.join(self._doxygen_url,
                                    'class{}.html'.format(nodes[0].class_name))

    def _code(self):
        """
        Return the GitHub/GitLab addresses for the associated C/h files.

        Args:
          repo_url[str]: Web address to use as the base for creating the edit link
        """
        out = set()
        nodes = self._syntax.findall('/' + self.node.name)
        for node in nodes:
            if isinstance(node, common.nodes.ObjectNode):
                if node.class_name in self._database:
                    for item in self._database[node.class_name]:
                        out.add((item.remote, item.filename, os.path.basename(item.filename)))
        return out
