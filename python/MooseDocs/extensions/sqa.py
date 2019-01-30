#pylint: disable=missing-docstring
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import re
import os
import codecs
import logging
import collections
import traceback
import anytree

import MooseDocs
from MooseDocs import common
from MooseDocs.common import exceptions
from MooseDocs.base import components
from MooseDocs.extensions import core, command, alert, floats, autolink, materialicon
from MooseDocs.tree import tokens, html

LOG = logging.getLogger(__name__)

def make_extension(**kwargs):
    return SQAExtension(**kwargs)

class SQAExtension(command.CommandExtension):

    @staticmethod
    def defaultConfig():
        config = command.CommandExtension.defaultConfig()
        config['directories'] = ([os.path.join(MooseDocs.ROOT_DIR, 'test', 'tests')],
                                 "List of directories used to build requirements.")
        config['specs'] = (['tests'], "List of test specification names to use for building " \
                                      "requirements.")
        config['requirement-groups'] = (dict(), "Allows requirement group names to be changed.")
        return config

    def __init__(self, *args, **kwargs):
        command.CommandExtension.__init__(self, *args, **kwargs)

        #NOTE: This is too slow to perform on reinit()
        self.__requirements = common.get_requirements(self.get('directories'), self.get('specs'))

        # Storage for requirement matrix counting (see SQARequirementMatricCommand)
        self.__counts = collections.defaultdict(int)

    @property
    def requirements(self):
        """Return the requirements dictionary."""
        return self.__requirements

    def preExecute(self, root):
        """Reset counts."""
        self.__counts.clear()

    def increment(self, key):
        """Increment and return count for requirements matrix."""
        self.__counts[key] += 1
        return self.__counts[key]

    def extend(self, reader, renderer):
        self.requires(core, command, alert, floats, core, materialicon)

        self.addCommand(reader, SQATemplateLoadCommand())
        self.addCommand(reader, SQATemplateItemCommand())
        self.addCommand(reader, SQARequirementsCommand())
        self.addCommand(reader, SQADocumentItemCommand())
        self.addCommand(reader, SQACrossReferenceCommand())
        self.addCommand(reader, SQARequirementsMatrixCommand())
        self.addCommand(reader, SQAVerificationCommand())

        renderer.add('SQATemplateItem', RenderSQATemplateItem())
        renderer.add('SQARequirementMatrix', RenderSQARequirementMatrix())
        renderer.add('SQARequirementMatrixItem', RenderSQARequirementMatrixItem())
        renderer.add('SQARequirementMatrixHeading', RenderSQARequirementMatrixHeading())

SQADocumentItem = tokens.newToken('SQADocumentItem', key=u'')
SQATemplateItem = tokens.newToken('SQATemplateItem', key=u'')
SQARequirementMatrix = tokens.newToken('SQARequirementMatrix')
SQARequirementMatrixItem = tokens.newToken('SQARequirementMatrixItem',
                                           label=u'',
                                           satisfied=True)
SQARequirementMatrixHeading = tokens.newToken('SQARequirementMatrixHeading')
SQAVandVMatrixItem = tokens.newToken('SQAVandVMatrixItem')
SQARequirementCrossReference = tokens.newToken('SQARequirementCrossReference')

class SQARequirementsCommand(command.CommandComponent):
    COMMAND = 'sqa'
    SUBCOMMAND = 'requirements'

    @staticmethod
    def defaultSettings():
        config = command.CommandComponent.defaultSettings()
        config['link'] = (True, "Enable/disable the linking of test specifications and " \
                                 "test files.")
        config['link-spec'] = (True, "Enable/disable the link of the test specification only, " \
                               "the 'link' setting must be true.")
        config['link-design'] = (True, "Enable/disable the link of the test design only, " \
                                 "the 'link' setting must be true.")
        config['link-issues'] = (True, "Enable/disable the link of the test issues only, " \
                                 "the 'link' setting must be true.")
        config['link-prerequisites'] = (True, "Enable/disable the link of the test prerequisites, "\
                                        "the 'link' setting must be true.")

        return config

    def createToken(self, parent, info, page):
        group_map = self.extension.get('requirement-groups')
        for group, requirements in self.extension.requirements.iteritems():
            group = group_map.get(group, group.replace('_', ' ').title())

            matrix = SQARequirementMatrix(parent)
            SQARequirementMatrixHeading(matrix, string=unicode(group))

            for req in requirements:
                self._addRequirement(matrix, info, page, req, requirements)

        return parent

    def _addRequirement(self, parent, info, page, req, requirements):
        item = SQARequirementMatrixItem(parent,
                                        label=unicode(req.label),
                                        satisfied=req.satisfied,
                                        id_=req.path)

        self.reader.tokenize(item, req.text, page, MooseDocs.INLINE, info.line, report=False)
        for token in anytree.PreOrderIter(item):
            if token.name == 'ErrorToken':
                msg = common.report_error("Failed to tokenize SQA requirement.",
                                          req.filename,
                                          req.text_line,
                                          req.text,
                                          token['traceback'],
                                          u'SQA TOKENIZE ERROR')
                LOG.critical(msg)


        if self.settings['link']:
            if self.settings['link-spec']:
                p = core.Paragraph(item)
                tokens.String(p, content=u'Specification: ')

                with codecs.open(req.filename, encoding='utf-8') as fid:
                    content = fid.read()

                floats.create_modal_link(p,
                                         title=req.filename,
                                         content=core.Code(None, language=u'text', content=content),
                                         string=u"{}:{}".format(req.path, req.name))

            if self.settings['link-design'] and req.design:
                p = core.Paragraph(item)
                tokens.String(p, content=u'Design: ')
                for design in req.design:
                    autolink.AutoLink(p, page=unicode(design))

            if self.settings['link-issues'] and req.issues:
                p = core.Paragraph(item)
                tokens.String(p, content=u'Issue(s): ')
                for issue in req.issues:
                    if issue.startswith('#'):
                        url = u"https://github.com/idaholab/moose/issues/{}".format(issue[1:])
                    else:
                        url = u"https://github.com/idaholab/moose/commit/{}".format(issue[1:])
                    core.Link(p, url=url, string=unicode(issue))
                    core.Space(p)

            if self.settings['link-prerequisites'] and req.prerequisites:
                labels = []
                for prereq in req.prerequisites:
                    for other in requirements:
                        if other.name == prereq:
                            labels.append(other.label)

                p = core.Paragraph(item)
                tokens.String(p, content=u'Prerequisites: {}'.format(' '.join(labels)))

class SQACrossReferenceCommand(SQARequirementsCommand):
    COMMAND = 'sqa'
    SUBCOMMAND = 'cross-reference'

    def createToken(self, parent, info, page):
        design = collections.defaultdict(list)
        for requirements in self.extension.requirements.itervalues():
            for req in requirements:
                for d in req.design:
                    try:
                        node = self.translator.findPage(d)
                        design[node].append(req)
                    except exceptions.MooseDocsException:
                        msg = "Failed to locate the design page '{}'".format(d)
                        LOG.critical(common.report_error(msg,
                                                         req.filename,
                                                         req.design_line,
                                                         ' '.join(req.design),
                                                         traceback.format_exc(),
                                                         'SQA ERROR'))

        for node, requirements in design.iteritems():
            matrix = SQARequirementMatrix(parent)
            heading = SQARequirementMatrixHeading(matrix)
            autolink.AutoLink(heading, page=unicode(node.local))
            for req in requirements:
                self._addRequirement(matrix, info, page, req, requirements)

        return parent

class SQARequirementsMatrixCommand(command.CommandComponent):
    COMMAND = 'sqa'
    SUBCOMMAND = 'requirements-matrix'
    COUNT = 1

    @staticmethod
    def defaultSettings():
        config = command.CommandComponent.defaultSettings()
        config['prefix'] = (None, "The letter prefix (e.g., 'P' for performance) for the type of "
                                  "requirement.")
        config['heading'] = (None, "Requirement matrix heading.")
        return config

    def createToken(self, parent, info, page):
        content = info['block'] if 'block' in info else info['inline']

        if self.settings['prefix'] is None:
            msg = "The 'prefix' option is required."
            raise exceptions.MooseDocsException(msg)

        # Extract the unordered list
        self.reader.tokenize(parent, content, page, MooseDocs.BLOCK, info.line)
        ul = parent.children[-1]
        ul.parent = None

        # Check the list type
        if ul.name != 'UnorderedList':
            print ul.name
            msg = "The content is required to be an unordered list (i.e., use '-')."
            raise exceptions.MooseDocsException(msg)

        # Build the matrix
        prefix = self.settings['prefix']
        label = u'{}{:d}'.format(prefix, self.extension.increment(prefix))
        matrix = SQARequirementMatrix(parent)

        heading = self.settings['heading']
        if heading is not None:
            head = SQARequirementMatrixHeading(matrix)
            self.reader.tokenize(head, heading, page, MooseDocs.INLINE, info.line)

        for i, item in enumerate(ul.children):
            matrix_item = SQARequirementMatrixItem(matrix, label=u'{}.{:d}'.format(label, i))
            for child in item:
                child.parent = matrix_item

        return parent

class SQATemplateLoadCommand(command.CommandComponent):
    COMMAND = 'sqa'
    SUBCOMMAND = 'load'

    @staticmethod
    def defaultSettings():
        config = command.CommandComponent.defaultSettings()
        config['template'] = (None, "The name of the template to load.")
        return config

    def createToken(self, parent, info, page):

        #TODO: make root path a config item in extension
        location = os.path.join(MooseDocs.MOOSE_DIR, 'framework', 'doc', 'templates', 'sqa',
                                self.settings['template'])

        if not os.path.exists(location):
            msg = "The template file does not exist: {}."
            raise exceptions.MooseDocsException(msg, location)

        with codecs.open(location, 'r', encoding='utf-8') as fid:
            content = fid.read()


        # Replace key/value arguments
        template_args = info['inline'] if 'inline' in info else info['block']
        _, key_values = common.match_settings(dict(), template_args)

        def sub(match):
            key = match.group('key')
            if key not in key_values:
                msg = "The template argument '{}' was not defined in the !sqa load command."
                raise exceptions.MooseDocsException(msg, key)

            return key_values[key]

        content = re.sub(r'{{(?P<key>.*?)}}', sub, content)

        # Tokenize the template
        self.reader.tokenize(parent, content, page, line=info.line)

        return parent

class SQATemplateItemCommand(command.CommandComponent):
    COMMAND = 'sqa'
    SUBCOMMAND = 'template'

    @staticmethod
    def defaultSettings():
        config = command.CommandComponent.defaultSettings()
        config['key'] = (None, "The name of the template item which the content is to replace.")
        config['heading'] = (None, "The section heading (optional).")
        config['heading-level'] = (3, "The heading level, if 'heading' is used.")
        config['required'] = (True, "The section is required.")
        return config

    def createToken(self, parent, info, page):
        key = self.settings['key']
        item = SQATemplateItem(parent, key=key)

        heading = self.settings.get('heading', None)
        if heading:
            item.heading = core.Heading(None, level=int(self.settings['heading-level']), id_=key)
            self.reader.tokenize(item.heading, heading, page, line=info.line)

        return item

class SQADocumentItemCommand(command.CommandComponent):
    COMMAND = 'sqa'
    SUBCOMMAND = 'item'

    @staticmethod
    def defaultSettings():
        config = command.CommandComponent.defaultSettings()
        config['key'] = (None, "The name of the template item which the content is to replace.")
        return config

    def createToken(self, parent, info, page):
        return SQADocumentItem(parent, key=self.settings['key'])

class SQAVerificationCommand(command.CommandComponent):
    COMMAND = 'sqa'
    SUBCOMMAND = ('verification', 'validation')

    @staticmethod
    def defaultSettings():
        config = command.CommandComponent.defaultSettings()
        return config

    def createToken(self, parent, info, page):

        matrix = SQARequirementMatrix(parent)
        for requirements in self.extension.requirements.itervalues():
            for req in requirements:
                if getattr(req, info['subcommand']):
                    self._addRequirement(matrix, info, page, req)

        return parent

    def _addRequirement(self, parent, info, page, req):
        item = SQARequirementMatrixItem(parent,
                                        label=unicode(req.label),
                                        satisfied=req.satisfied,
                                        id_=req.path)


        self.reader.tokenize(item, req.text, page, MooseDocs.INLINE, info.line, report=False)
        for token in anytree.PreOrderIter(item):
            if token.name == 'ErrorToken':
                msg = common.report_error("Failed to tokenize SQA requirement.",
                                          req.filename,
                                          req.text_line,
                                          req.text,
                                          token['traceback'],
                                          u'SQA TOKENIZE ERROR')
                LOG.critical(msg)

        p = core.Paragraph(item)
        tokens.String(p, content=u'Specification: ')

        with codecs.open(req.filename, encoding='utf-8') as fid:
            content = fid.read()
            floats.create_modal_link(p,
                                     string=u"{}:{}".format(req.path, req.name),
                                     content=core.Code(None, language=u'text', content=content),
                                     title=unicode(req.filename))

        p = core.Paragraph(item)
        tokens.String(p, content=u'Details: ')
        filename = u'{}/{}.md'.format(req.path, req.name)
        autolink.AutoLink(p, page=unicode(filename))

class RenderSQATemplateItem(components.RenderComponent):

    def createHTML(self, parent, token, page):
        pass

    def createMaterialize(self, parent, token, page):

        key = token['key']
        func = lambda n: (n.name == 'SQADocumentItem') and (n['key'] == key)
        replacement = anytree.search.find(token.root, filter_=func, maxlevel=2)

        if replacement:

            self.renderer.render(parent, replacement, page)

            # Remove item so it doesn't render again
            replacement.parent = None
            for child in replacement:
                child.parent = None
#
        else:
            filename = page.local
            err = alert.AlertToken(None, brand=u'error')
            alert_title = alert.AlertTitle(err,
                                           brand=u'error',
                                           string=u'Missing Template Item "{}"'.format(key))
            alert_content = alert.AlertContent(err, brand=u'error')

            modal_content = tokens.Token(None)
            core.Paragraph(modal_content,
                           string=u"The document must include the \"{0}\" template item, this can "\
                           u"be included by add adding the following to the markdown " \
                           u"file ({1}):".format(key, filename))

            core.Code(modal_content,
                      code=u"!sqa! item key={0}\nInclude text (in MooseDocs format) " \
                      u"regarding the \"{0}\" template item here.\n" \
                      u"!sqa-end!".format(key))

            link = floats.create_modal_link(alert_title,
                                            title=u'Missing Template Item "{}"'.format(key),
                                            content=modal_content)
            materialicon.IconToken(link, icon=u'help_outline', class_=u'material-icons moose-help')


            for child in token.children:
                child.parent = alert_content

            self.renderer.render(parent, err, page)

class RenderSQARequirementMatrix(core.RenderUnorderedList):
    def createMaterialize(self, parent, token, page):
        return html.Tag(parent, 'ul', class_="moose-requirements collection with-header")

class RenderSQARequirementMatrixItem(core.RenderListItem):
    def createMaterialize(self, parent, token, page): #pylint: disable=no-self-use,unused-argument
        li = html.Tag(parent, 'li', class_="collection-item", **token.attributes)
        num = html.Tag(li, 'span', string=token['label'], class_='moose-requirement-number')

        id_ = token.get('id', None)
        if id_:
            num.addClass('tooltipped')
            num['data-tooltip'] = id_

        if not token['satisfied']:
            num = html.Tag(li, 'i', string=u'block',
                           class_='material-icons moose-requirement-unsatisfied')

        return html.Tag(li, 'span', class_='moose-requirement-content')

class RenderSQARequirementMatrixHeading(core.RenderListItem):

    def createMaterialize(self, parent, token, page):
        return html.Tag(parent, 'li', class_='collection-header')
