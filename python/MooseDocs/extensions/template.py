#pylint: disable=missing-docstring
####################################################################################################
#                                    DO NOT MODIFY THIS HEADER                                     #
#                   MOOSE - Multiphysics Object Oriented Simulation Environment                    #
#                                                                                                  #
#                              (c) 2010 Battelle Energy Alliance, LLC                              #
#                                       ALL RIGHTS RESERVED                                        #
#                                                                                                  #
#                            Prepared by Battelle Energy Alliance, LLC                             #
#                               Under Contract No. DE-AC07-05ID14517                               #
#                               With the U. S. Department of Energy                                #
#                                                                                                  #
#                               See COPYRIGHT for full restrictions                                #
####################################################################################################
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

    def arguments(self, template_args, text): #pylint: disable=no-self-use
        """
        Method for modifying the template arguments to be applied to the jinja2 templates engine.

        Args:
            template_args[dict]: Template arguments to be applied to jinja2 template.
            text[str]: Convert markdown to be applied via the 'content' template argument.
        """
        template_args['content'] = text

        if 'navigation' in template_args:
            template_args['navigation'] = \
                MooseDocs.yaml_load(MooseDocs.abspath(template_args['navigation']))

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
            with open(MooseDocs.abspath(filename), 'r') as fid:
                out += [fid.read().strip('\n')]
        return '\n'.join(out)

    @staticmethod
    def _imageLinks(node, soup):
        """
        Makes images links relative
        """
        for img in soup('img'):
            if 'src' in img.attrs:
                img['src'] = node.relpath(img['src'])

    @staticmethod
    def _markdownLinks(node, soup):
        """
        Performs auto linking of markdown files.
        """
        def finder(node, desired, pages):
            """
            Locate nodes for the 'desired' filename
            """
            if node.source() and node.source().endswith(desired):
                pages.append(node)
            for child in node:
                finder(child, desired, pages)
            return pages

        # Loop over <a> tags and update links containing .md to point to .html
        for link in soup('a'):
            href = link.get('href')
            if href and (not href.startswith('http')) and ('.md' in href):

                # Split filename from section link (#)
                parts = href.split('#')

                # Populate the list of found files
                found = []
                finder(node.root(), parts[0], found)

                # Error if file not found or if multiple files found
                if not found:
                    LOG.error('Failed to locate page for markdown file %s in %s',
                              href, node.source())
                    link['class'] = 'moose-bad-link'
                    continue

                elif len(found) > 1:
                    msg = 'Found multiple pages matching the supplied markdown file {} in {}:' \
                          .format(href, node.source())
                    for f in found:
                        msg += '\n    {}'.format(f.source())
                    LOG.error(msg)

                # Update the link with the located page
                url = node.relpath(found[0].url())
                if len(parts) == 2:
                    url += '#' + parts[1]
                LOG.debug('Converting link: %s --> %s', href, url)
                link['href'] = url


class TemplatePostprocessor(TemplatePostprocessorBase):
    """
    A template extension that works with the 'MooseDocs.extensions.app_syntax' extension.
    """
    def __init__(self, *args, **kwargs):
        super(TemplatePostprocessor, self).__init__(*args, **kwargs)

        # The 'MooseDocs.extensions.app_syntax' are required
        self.markdown.requireExtension(AppSyntaxExtension)

        # Store the MooseApplicationSyntax object for use later.
        ext = self.markdown.getExtension(AppSyntaxExtension)
        self._syntax = ext.syntax

    def globals(self, env):
        """
        Add MOOSE syntax related functions for the template.
        """
        super(TemplatePostprocessor, self).globals(env)
        env.globals['editMarkdown'] = self._editMarkdown
        env.globals['mooseCode'] = self._code

    def arguments(self, template_args, text):
        """
        Add MOOSE syntax related arguments to the template arguments.
        """
        super(TemplatePostprocessor, self).arguments(template_args, text)
        template_args['tableofcontents'] = self._tableofcontents(text)
        template_args['doxygen'] = self._doxygen()

    @staticmethod
    def _tableofcontents(text, level='h2'):
        """
        Returns the h2 (default) tags for the supplied html text.
        """
        soup = bs4.BeautifulSoup(text, 'html.parser')
        for tag in soup.find_all(level):
            if 'id' in tag.attrs and tag.contents:
                yield (tag.contents[0], tag.attrs['id'])

    def _editMarkdown(self, repo_url):
        """
        Return the url to the markdown file for this object.
        """
        return os.path.join(repo_url, 'edit', 'devel', MooseDocs.relpath(self.node.source()))

    def _doxygen(self):
        """
        Return the doxygen link, if it exists.
        """
        for syntax in self._syntax.itervalues():
            for obj in syntax.objects().itervalues():
                if obj.name == self.node.name():
                    return syntax.doxygen(obj.name)

    def _code(self, repo_url):
        """
        Return the GitHub/GitLab addresses for the associated C/h files.

        Args:
          repo_url[str]: Web address to use as the base for creating the edit link
        """
        info = []
        for syntax in self._syntax.itervalues():
            for obj in syntax.objects().itervalues():
                if obj.name == self.node.name():
                    info.append(obj)
            for obj in syntax.actions().itervalues():
                if obj.name == self.node.name():
                    info.append(obj)

        output = []
        for obj in info:
            for filename in obj.code:
                rel_filename = MooseDocs.relpath(filename)
                output.append((os.path.basename(rel_filename),
                               os.path.join(repo_url, 'blob', 'master', rel_filename)))

        return output
