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
import shutil
import logging

import livereload
import bs4

import MooseDocs
from MarkdownNode import MarkdownNode

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
    parser.add_argument('--output', '-o', default=None, type=str,
                        help="The default html file to create, defaults to input filename with "
                             "html extension.")
    parser.add_argument('--template', type=str, default='presentation.html',
                        help="The template html file to utilize (default: %(default)s).")
    parser.add_argument('--title', type=str, default="MOOSE Presentation",
                        help="The title of the document.")
    css_default = [os.path.join(MooseDocs.MOOSE_DIR, 'docs', 'css', 'moose.css')]
    parser.add_argument('--css', type=str, nargs='+', default=css_default,
                        help="A list of additional css files to inject into the presentation html "
                             "file (%(default)s).")

    scripts_default = [os.path.join(MooseDocs.MOOSE_DIR, 'docs', 'css', 'moose.css')]
    parser.add_argument('--scripts', type=str, nargs='+', default=scripts_default,
                        help="A list of additional js files to inject into the presentation html "
                             "file (%(default)s).")
    return presentation

class PresentationBuilder(MarkdownNode):
    """
    Adds a copy of image files to the destination directory.
    """
    def build(self, lock=None):
        content = self.convert()
        soup = bs4.BeautifulSoup(content, 'html.parser')
        for img in soup('img'):
            name = os.path.basename(img['src'])
            dest = os.path.join(self.path(), name)
            dirname = os.path.dirname(dest)
            if not os.path.exists(dirname):
                os.makedirs(dirname)
            LOG.debug('Copying images: %s to %s', img['src'], dest)
            shutil.copyfile(img['src'], dest)
            img['src'] = name

        self.write(soup.prettify())

def presentation(config_file=None, md_file=None, serve=None, port=None, host=None,
                 template=None, **template_args):
    """
    MOOSE markdown presentation blaster.
    """

    # Load the YAML configuration file
    config = MooseDocs.load_config(config_file, template=template, template_args=template_args)
    parser = MooseDocs.MooseMarkdown(extensions=config.keys(), extension_configs=config)

    site_dir, _ = os.path.splitext(md_file)
    if not os.path.isdir(site_dir):
        os.mkdir(site_dir)
    root = PresentationBuilder(name='', markdown=md_file, parser=parser, site_dir=site_dir)
    root.build()

    if serve:
        server = livereload.Server()
        server.watch(md_file, root.build)
        server.serve(root=site_dir, host=host, port=port, restart_delay=0)
    return None
