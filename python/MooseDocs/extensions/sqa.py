#pylint: disable=missing-docstring
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import os
import re
import copy
import codecs
import logging
import collections
import traceback
import anytree

import MooseDocs
from MooseDocs import common
from MooseDocs.common import exceptions
from MooseDocs.base import components, LatexRenderer
from MooseDocs.extensions import core, command, alert, floats, autolink, materialicon
from MooseDocs.tree import tokens, html, latex

LOG = logging.getLogger(__name__)

def make_extension(**kwargs):
    return SQAExtension(**kwargs)

SQARequirementMatrix = tokens.newToken('SQARequirementMatrix')
SQARequirementMatrixItem = tokens.newToken('SQARequirementMatrixItem', requirement=None)
SQARequirementMatrixListItem = tokens.newToken('SQARequirementMatrixListItem', label=None)
SQARequirementText = tokens.newToken('SQARequirementText')
SQARequirementDesign = tokens.newToken('SQARequirementDesign', design=[])
SQARequirementIssues = tokens.newToken('SQARequirementIssues', issues=[])
SQARequirementSpecification = tokens.newToken('SQARequirementSpecification',
                                              spec_name=None, spec_path=None)
SQARequirementPrequisites = tokens.newToken('SQARequirementPrequisites', specs=[])


SQARequirementMatrixHeading = tokens.newToken('SQARequirementMatrixHeading')
SQAVandVMatrixItem = tokens.newToken('SQAVandVMatrixItem')
SQARequirementCrossReference = tokens.newToken('SQARequirementCrossReference')

LATEX_REQUIREMENT = """
\\DeclareDocumentEnvironment{Requirement}{m}{%
  \\begin{minipage}[t]{0.08\\textwidth}%
    #1
  \\end{minipage}
  \\hfill
  \\begin{minipage}[t]{0.9\\textwidth}%
}{%
  \\end{minipage}
}
"""

class SQAExtension(command.CommandExtension):

    @staticmethod
    def defaultConfig():
        config = command.CommandExtension.defaultConfig()
        config['url'] = (u"https://github.com",
                         "Repository for linking issues and commits.")
        config['repo'] = (u"idaholab/moose",
                          "The default repository location to append to the given url.")
        config['directories'] = ([os.path.join(MooseDocs.ROOT_DIR, 'test', 'tests')],
                                 "List of directories used to build requirements.")
        config['specs'] = (['tests'], "List of test specification names to use for building " \
                                      "requirements.")
        config['requirement-groups'] = (dict(), "Allows requirement group names to be changed.")
        return config

    def __init__(self, *args, **kwargs):
        command.CommandExtension.__init__(self, *args, **kwargs)

        # Build the w to perform on reinit()
        directories = []
        for d in self.get('directories'):
            path = os.path.join(MooseDocs.ROOT_DIR, d)
            if not os.path.isdir(path):
                msg = "Input directory does not exist: %s"
                LOG.error(msg, path)
            directories.append(path)

        # Create requirement database
        self.__requirements = common.get_requirements(directories, self.get('specs'))

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

        self.addCommand(reader, SQARequirementsCommand())
        self.addCommand(reader, SQACrossReferenceCommand())
        self.addCommand(reader, SQARequirementsMatrixCommand())
        self.addCommand(reader, SQAVerificationCommand())

        renderer.add('SQARequirementMatrix', RenderSQARequirementMatrix())
        renderer.add('SQARequirementMatrixItem', RenderSQARequirementMatrixItem())
        renderer.add('SQARequirementMatrixListItem', RenderSQARequirementMatrixListItem())
        renderer.add('SQARequirementMatrixHeading', RenderSQARequirementMatrixHeading())
        renderer.add('SQARequirementText', RenderSQARequirementText())
        renderer.add('SQARequirementDesign', RenderSQARequirementDesign())
        renderer.add('SQARequirementIssues', RenderSQARequirementIssues())
        renderer.add('SQARequirementSpecification', RenderSQARequirementSpecification())
        renderer.add('SQARequirementPrequisites', RenderSQARequirementPrequisites())

        if isinstance(renderer, LatexRenderer):
            renderer.addPackage('xcolor')
            renderer.addPreamble(LATEX_REQUIREMENT)

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

        item = SQARequirementMatrixItem(parent, requirement=req)
        text = SQARequirementText(item)

        self.reader.tokenize(text, req.text, page, MooseDocs.INLINE, info.line, report=False)
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
                p = SQARequirementSpecification(item, spec_path=req.path, spec_name=req.name)

                with codecs.open(req.filename, encoding='utf-8') as fid:
                    content = fid.read()
                floats.create_modal_link(p,
                                         title=req.filename,
                                         content=core.Code(None, language=u'text', content=content),
                                         string=u"{}:{}".format(req.path, req.name))

            if self.settings['link-design'] and req.design:
                p = SQARequirementDesign(item, design=req.design)

            if self.settings['link-issues'] and req.issues:
                p = SQARequirementIssues(item, issues=req.issues)

            if self.settings['link-prerequisites'] and req.prerequisites:
                labels = []
                for prereq in req.prerequisites:
                    for other in requirements:
                        if (other.name == prereq) and (other.path == req.path):
                            labels.append((other.path, other.name, other.label))

                SQARequirementPrequisites(item, specs=labels)

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
            matrix_item = SQARequirementMatrixListItem(matrix, label=u'{}.{:d}'.format(label, i))
            for child in item:
                child.parent = matrix_item

        return parent


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
                if getattr(req, info['subcommand']) is not None:
                    self._addRequirement(matrix, info, page, req)

        return parent

    def _addRequirement(self, parent, info, page, req):
        item = SQARequirementMatrixItem(parent, requirement=req)
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
        filename = getattr(req, info['subcommand'])
        autolink.AutoLink(p, page=unicode(filename))


class RenderSQARequirementMatrix(core.RenderUnorderedList):
    def createMaterialize(self, parent, token, page):
        return html.Tag(parent, 'ul', class_="moose-requirements collection with-header")

    def createLatex(self, parent, token, page):
        return parent

class RenderSQARequirementMatrixItem(core.RenderListItem):
    def createMaterialize(self, parent, token, page): #pylint: disable=no-self-use,unused-argument

        attributes = copy.copy(token.attributes)
        req = attributes.pop('requirement')
        if req is None:
            print token

        id_ = u'{}:{}'.format(req.path, req.name)

        li = html.Tag(parent, 'li', class_="collection-item", **attributes)
        num = html.Tag(li, 'span', string=req.label, id_=id_, class_='moose-requirement-number')
        num.addClass('tooltipped')
        num['data-tooltip'] = id_

        if not req.satisfied:
            num = html.Tag(li, 'i', string=u'block',
                           class_='material-icons moose-requirement-unsatisfied')

        return html.Tag(li, 'span', class_='moose-requirement-content')

    def createLatex(self, parent, token, page):
        args = [latex.Brace(string=token['requirement'].label)]
        return latex.Environment(parent, 'Requirement', args=args)

class RenderSQARequirementMatrixListItem(RenderSQARequirementMatrixItem):
    def createMaterialize(self, parent, token, page): #pylint: disable=no-self-use,unused-argument

        attributes = copy.copy(token.attributes)
        li = html.Tag(parent, 'li', class_="collection-item", **attributes)
        html.Tag(li, 'span', string=token['label'], class_='moose-requirement-number')
        return html.Tag(li, 'span', class_='moose-requirement-content')

    def createLatex(self, parent, token, page):
        args = [latex.Brace(string=token['label'])]
        return latex.Environment(parent, 'Requirement', args=args)

class RenderSQARequirementMatrixHeading(core.RenderListItem):

    def createMaterialize(self, parent, token, page):
        return html.Tag(parent, 'li', class_='collection-header')

    def createLatex(self, parent, token, page):
        prefix = token.siblings[0]['requirement'].label.split('.')[0]
        return latex.Command(parent, 'section*', string=prefix + ':~', escape=False)

class RenderSQARequirementText(components.RenderComponent):
    def createHTML(self, parent, token, page):
        return parent

    def createLatex(self, parent, token, page):
        return parent

class RenderSQARequirementDesign(autolink.RenderLinkBase):

    def findDesign(self, design, req):
        node = self.translator.findPage(design, throw_on_zero=False)
        if node is None:
            msg = "Unable to locate the design page: {}\n    {}:{}" \
                  .format(design, req.filename, req.design_line)
            LOG.error(msg)
            return None
        return node

    def createHTML(self, parent, token, page):
        p = html.Tag(parent, 'p', string=u'Design: ', class_='moose-sqa-items')
        for design in token['design']:
            node = self.findDesign(design, token.parent['requirement'])
            if node is not None:
                link = autolink.AutoLink(None, page=page)
                link.info = token.info
                self.createHTMLHelper(p, link, page, node)
            else:
                html.Tag(p, 'a', string=unicode(design), class_='moose-error')

    def createLatex(self, parent, token, page):

        latex.Command(parent, 'newline', start='\n', end='\n')
        latex.String(parent, content='Design:~', escape=False)

        no_seperator = True
        for design in token['design']:
            if not no_seperator:
                latex.String(parent, content='; ')
                no_seperator = False

            node = self.findDesign(design, token)
            if node:
                link = autolink.AutoLink(None, page=page)
                link.info = token.info
                self.createLatexHelper(parent, link, page, node)
            else:
                latex.Command(parent, 'textcolor', args=[latex.Brace(string='red')], string=design)

class RenderSQARequirementIssues(components.RenderComponent):

    def getURL(self, issue, token):
        url = None
        base = self.extension['url'].rstrip('/')
        repo = self.extension['repo'].strip('/')

        match = re.search(r"(?P<repo>\w+/\w+)#(?P<issue>[0-9]+)", issue)
        if match:
            url = u"{}/{}/issues/{}".format(base, match.group('repo'), match.group('issue'))
        elif issue.startswith('#'):
            url = u"{}/{}/issues/{}".format(base, repo, issue[1:])
        elif re.search(r'\A[0-9a-f]{10,40}\Z', issue):
            url = u"{}/{}/commit/{}".format(base, repo, issue[1:])

        if (url is None) and (issue != u''):
            req = token.parent['requirement']
            msg = "Unknown issue number or commit (commit SHA-1 must be at least 10 digits): "\
                  "{}\n    {}:{}".format(issue, req.filename, req.issues_line)
            LOG.error(msg)

        return url

    def createHTML(self, parent, token, page):

        p = html.Tag(parent, 'p', string=u'Issue(s): ', class_='moose-sqa-items')
        for issue in token['issues']:

            url = self.getURL(issue, token)
            if url is None:
                html.Tag(p, 'a', string=issue, class_='moose-error')
            else:
                html.Tag(p, 'a', href=url, string=unicode(issue))

    def createLatex(self, parent, token, page):

        latex.Command(parent, 'newline', start='\n', end='\n')
        latex.String(parent, content='Issue(s):~', escape=False)
        no_seperator = True

        for issue in token['issues']:
            if not no_seperator:
                latex.String(parent, content='; ')
                no_seperator = False

            url = self.getURL(issue, token)
            if url is None:
                latex.Command(parent, 'textcolor', args=[latex.Brace(string='red')], string=issue)
            else:
                latex.Command(parent, 'href', args=[latex.Brace(string=url)], string=unicode(issue))

class RenderSQARequirementSpecification(components.RenderComponent):
    def createHTML(self, parent, token, page):
        return html.Tag(parent, 'p', string=u'Specification: ')

    def createLatex(self, parent, token, page):
        latex.Command(parent, 'newline', start='\n', end='\n')
        spec = 'Specification: {}:{}'.format(token['spec_path'], token['spec_name'])
        latex.String(parent, content=spec)

class RenderSQARequirementPrequisites(components.RenderComponent):
    def createHTML(self, parent, token, page):
        p = html.Tag(parent, 'p', string=u'Prerequisite(s): ', class_='moose-sqa-items')

        for path, name, label in token['specs']:
            url = u'#{}:{}'.format(path, name)
            html.Tag(p, 'a', href=url, string=label)

    def createLatex(self, parent, token, page):
        latex.Command(parent, 'newline', start='\n', end='\n')
        latex.String(parent, content='Prerequisite(s):~', escape=False)

        labels = [label for _, _, label in token['specs']]
        latex.String(parent, content='; '.join(labels))
