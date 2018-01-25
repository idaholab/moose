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
import logging

import livereload

import MooseDocs
from MooseDocs.MooseMarkdown import MooseMarkdown
from MooseDocs.common import nodes, Builder

LOG = logging.getLogger(__name__)

def presentation_options(parser):
    """
    Command line options for the presentation generator.
    """
    parser.add_argument('md_file', type=str, help="The markdown file to convert to slides.")
    parser.add_argument('--serve', action='store_true',
                        help="Serve the presentation with live reloading.")
    parser.add_argument('--host', default='127.0.0.1', type=str,
                        help="The local host location for live web server (default: %(default)s).")
    parser.add_argument('--port', default='8000', type=str,
                        help="The local host port for live web server (default: %(default)s).")
    yaml_default = os.path.join(MooseDocs.MOOSE_DIR, 'docs', 'presentation.yml')
    parser.add_argument('--config-file', type=str, default=yaml_default,
                        help="The configuration file to use for building the documentation using "
                             "MOOSE. (Default: %(default)s)")
    parser.add_argument('--template', type=str, default='presentation.html',
                        help="The template html file to utilize (default: %(default)s).")
    parser.add_argument('--title', type=str, default="MOOSE Presentation",
                        help="The title of the document.")
    css_default = [os.path.join(MooseDocs.MOOSE_DIR, 'docs', 'content', 'css', 'moose.css')]
    parser.add_argument('--css', type=str, nargs='+', default=css_default,
                        help="A list of additional css files to inject into the presentation html "
                             "file (%(default)s).")

    scripts_default = [os.path.join(MooseDocs.MOOSE_DIR, 'docs', 'css', 'moose.css')]
    parser.add_argument('--scripts', type=str, nargs='+', default=scripts_default,
                        help="A list of additional js files to inject into the presentation html "
                             "file (%(default)s).")
    return presentation


class MarkdownPresentationNode(nodes.MarkdownFilePageNode):
    """
    A node based on a single markdown file for building presentations.

    Inputs:
        markdown[str]: The markdown file to convert.
    """
    def __init__(self, markdown):
        name = os.path.basename(markdown)[:-3]
        base = os.path.dirname(markdown)
        super(MarkdownPresentationNode, self).__init__(name, base=base)

class PresentationBuilder(Builder):
    """
    A builder for presentation markdown files.

    Inputs:
        md_file[str]: The path of the markdown file relative to the repository root directory.
        kwargs[dict]: See Builder
    """

    def __init__(self, md_file=None, **kwargs):
        kwargs['site_dir'] = os.path.join(MooseDocs.ROOT_DIR, os.path.dirname(md_file))
        super(PresentationBuilder, self).__init__(**kwargs)
        self._md_file = md_file

    def buildNodes(self):
        """
        Create the markdown node objects to build.
        """
        # Check if file exists
        full = os.path.join(MooseDocs.ROOT_DIR, self._md_file)
        if not os.path.isfile(full):
            raise IOError("The presentation markdown file does not exist: {}.".format(full))

        # Return the node for converting.
        return MarkdownPresentationNode(self._md_file)

    def rootDirectory(self):
        """
        Return the directory to the presentation index.html
        """
        return os.path.join(self._site_dir, self._root.destination)

def presentation(config_file=None, md_file=None, serve=None, port=None, host=None, template=None,
                 **template_args):
    """
    MOOSE markdown presentation blaster.
    """

    # The markdown file is provided via the command line, thus it is provided relative to the
    # current working directory. The internals of MooseDocs are setup to always work from the
    # repository root directory (i.e., MooseDocs.ROOT_DIR), thus the path of this file must be
    # converted to be relative to MooseDocs.ROOT_DIR.
    md_file = os.path.relpath(os.path.abspath(md_file), MooseDocs.ROOT_DIR)

    # Create the markdown parser
    config = MooseDocs.load_config(config_file, template=template, template_args=template_args)
    parser = MooseMarkdown(config)

    # Build the html
    builder = PresentationBuilder(md_file=md_file, parser=parser)
    builder.init()
    builder.build(num_threads=1)

    if serve:
        server = livereload.Server()
        server.watch(os.path.join(MooseDocs.ROOT_DIR, md_file),
                     lambda: builder.build(num_threads=1))
        server.serve(root=builder.rootDirectory(), host=host, port=port, restart_delay=0)

    return 0
