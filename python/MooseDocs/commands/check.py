#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

"""Developer tools for MooseDocs."""
import os
import re
import collections
import logging

import anytree

from MooseDocs import common
from MooseDocs.tree import syntax
from MooseDocs.common import exceptions

LOG = logging.getLogger(__name__)

def command_line_options(subparser, parent):
    """Define the 'check' command."""

    parser = subparser.add_parser('check',
                                  parents=[parent],
                                  help='Syntax checking tools for MooseDocs.')

    parser.add_argument('--config', default='config.yml', help="The configuration file.")
    parser.add_argument('--generate', action='store_true',
                        help="When checking the application for complete documentation generate "
                             "any missing markdown documentation files.")
    parser.add_argument('--update', action='store_true',
                        help="When checking the application for complete documentation generate "
                             "any missing markdown documentation files and update the stubs for "
                             "files that have not been modified.")
    parser.add_argument('--dump', action='store_true',
                        help="Dump the complete MooseDocs syntax tree to the screen.")
    parser.add_argument('--object_prefix',
                        default=os.path.join('doc', 'content', 'source'),
                        help="The folder install prefix for MooseObject documentations pages.")
    parser.add_argument('--syntax_prefix',
                        default=os.path.join('doc', 'content', 'syntax'),
                        help="The folder install prefix for MooseObject documentations pages.")

def main(options):
    """./moosedocs check"""

    translator, _ = common.load_config(options.config)
    err = check(translator,
                dump=options.dump,
                update=options.update,
                generate=options.generate,
                object_prefix=options.object_prefix,
                syntax_prefix=options.syntax_prefix)
    return err

def check(translator,
          dump=False,
          update=False,
          generate=False,
          object_prefix=os.path.join('doc', 'content', 'source'),
          syntax_prefix=os.path.join('doc', 'content', 'syntax')):
    """Helper to all both main and build.py:main to perform check."""

    # Extract the syntax root node
    app_syntax = None
    extension = None
    for ext in translator.extensions:
        if ext.name == 'appsyntax':
            extension = ext
            extension.preExecute(translator.content)
            app_syntax = ext.syntax
            break

    if (extension is None) or (not extension.active):
        LOG.info("Syntax is disabled, skipping the check.")
        return 0

    if app_syntax is None:
        msg = "Failed to locate AppSyntaxExtension for the given configuration."
        raise exceptions.MooseDocsException(msg)

    # Dump the complete syntax for the application
    if dump:
        print(app_syntax)

    # The default build for any application creates the test app (e.g., FrogTestApp), therefore
    # the actual application name must be captured from this name to properly generate stub pages.
    # This is because the --generate option will only create stub pages for objects registered
    # within the running application, otherwise pages for all MOOSE and modules objects would
    # be generated if they were not included. However, for a given object the registered application
    # from the register macros is used, thus the compiled application used for building
    # documentation (FrogTestApp) and the object registration name (FrogApp) are always out of sync.
    # The simple fix is to change the name of the running application here.
    app_name = extension.apptype.replace('TestApp', 'App')

    # Perform check for all the nodes
    for node in anytree.PreOrderIter(app_syntax):
        if node.is_root or node.removed:
            continue
        elif isinstance(node, syntax.ObjectNode):
            _check_object_node(node, app_name, generate, update, object_prefix)
        elif isinstance(node, syntax.SyntaxNode):
            _check_syntax_node(node, app_name, generate, update, syntax_prefix)
        else:
            LOG.critical("Unexpected object type of %s, only %s and %s based types are supported",
                         type(node).__name__,
                         syntax.ObjectNode.__name__,
                         syntax.SyntaxNode.__name__)

    return 0

def _check_object_node(node, app_name, generate, update, prefix):
    """
    Check that required pages for supplied ObjectNode (i.e., MooseObject/Action).
    """
    idx = node.source().find('/src/')

    # If the markdown method returns None, it failed to locate the file, which throws an error
    md = node.markdown()
    if md is None:
        return
    filename = os.path.join(node.source()[:idx], prefix, md)

    not_exist = not os.path.isfile(filename)
    if not_exist and (not generate) and (not node.hidden):
        msg = "No documentation for %s.\n"
        msg += "    - The page should be located at %s.\n"
        msg += "    - It is possible to generate stub pages using " \
               "'./moosedocs.py check --generate'."
        LOG.error(msg, node.fullpath, filename)

    elif not_exist and generate and (app_name in node.groups):
        if not os.path.exists(os.path.dirname(filename)):
            os.makedirs(os.path.dirname(filename))
        LOG.info('Creating stub page for %s at %s.', node.fullpath, filename)
        with open(filename, 'w') as fid:
            content = _default_content(node)
            fid.write(content)

    elif not not_exist:
        _check_page_for_stub(node, app_name, filename, update)

def _check_syntax_node(node, app_name, generate, update, prefix):
    """
    Check that required pages for syntax exists (e.g., Adaptivity/index.md).
    """

    # Tuple for storing filename and existence
    FileInfo = collections.namedtuple('FileInfo', 'name exists') #pylint: disable=invalid-name

    # Build a set if information tuples to consider
    filenames = set()
    func = lambda n: isinstance(n, syntax.ActionNode) and not n.removed
    actions = anytree.search.PreOrderIter(node, filter_=func)
    for action in actions:
        idx = action.source().find('/src/')
        name = os.path.join(action.source()[:idx], prefix,
                            os.path.dirname(node.markdown()), 'index.md')
        filenames.add(FileInfo(name=name, exists=os.path.isfile(name)))

    # Case when no file exists
    not_exist = all(not f.exists for f in filenames)
    if not_exist and (not generate) and (not node.hidden):
        msg = "No documentation for {}.\n".format(node.fullpath)
        if len(filenames) == 1:
            msg += "    - The page should be located at {}.\n".format(filenames.pop().name)
            msg += "    - It is possible to generate stub pages using " \
                   "'./moosedocs.py check --generate'."
        else:
            msg += "    - The page should be located at one of the following locations:\n"
            for info in filenames:
                msg += "      {}\n".format(info.name)

        LOG.error(msg)

    # Case when when no file exists but --generate was provided
    elif not_exist and generate:
        if len(filenames) == 1:
            fname = filenames.pop().name
            LOG.info("Creating stub page for %s at %s.", node.fullpath, fname)
            if not os.path.exists(os.path.dirname(fname)):
                os.makedirs(os.path.dirname(fname))
            with open(fname, 'w') as fid:
                content = _default_content(node)
                fid.write(content)
        else:
            msg = "A stub page for {} cannot be created because the syntax definition location " \
                  "exists in multiple applications, the file can be located at any of the " \
                  "following locations.\n".format(node.fullpath)
            for info in filenames:
                msg += "      {}\n".format(info.name)
            LOG.error(msg)

    else:
        for info in filenames:
            if info.exists:
                _check_page_for_stub(node, app_name, info.name, update)

def _check_page_for_stub(node, app_name, filename, update):
    """
    Helper method to check if a page is a stub.
    """
    with open(filename, 'r') as fid:
        content = fid.read()

    if content and re.search(r'(stubs\/moose_(object|action|system).md.template)', content) or \
       ('MOOSE Documentation Stub' in content):
        if update and (app_name in node.groups):
            LOG.info("Updating stub page for %s in file %s.", node.fullpath, filename)
            with open(filename, 'w') as fid:
                content = _default_content(node)
                fid.write(content)
        elif not node.hidden:
            msg = "A MOOSE generated stub page for %s exists, but no content was " \
                  "added. Add documentation content to %s."
            LOG.warning(msg, node.fullpath, filename)

    elif content and node.hidden:
        msg = "A page for %s exists, but it is still listed as hidden."
        LOG.warning(msg, node.fullpath)


    elif content and node.hidden:
        msg = "A page for %s exists, but it is still listed as hidden."
        LOG.error(msg, node.fullpath)


def _default_content(node):
    """
    Markdown stub content.
    """
    if isinstance(node, syntax.SyntaxNode):
        tname = 'moose_system.md.template'
    elif isinstance(node, syntax.MooseObjectNode):
        tname = 'moose_object.md.template'
    elif isinstance(node, syntax.ActionNode):
        tname = 'moose_action.md.template'
    return '!template load file=stubs/{} name={} syntax={}'.format(tname, node.name, node.fullpath)
