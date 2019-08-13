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

import mooseutils
import MooseDocs
from MooseDocs import common
from MooseDocs.common import exceptions
from MooseDocs.base import components, LatexRenderer, HTMLRenderer
from MooseDocs.extensions import core, command, floats, autolink
from MooseDocs.tree import tokens, html, latex

LOG = logging.getLogger(__name__)

def make_extension(**kwargs):
    return SQAExtension(**kwargs)

SQARequirementMatrix = tokens.newToken('SQARequirementMatrix')
SQARequirementMatrixItem = tokens.newToken('SQARequirementMatrixItem', label=None, reqname=None,
                                           satisfied=True)
SQARequirementMatrixListItem = tokens.newToken('SQARequirementMatrixListItem', label=None)
SQARequirementText = tokens.newToken('SQARequirementText')
SQARequirementDesign = tokens.newToken('SQARequirementDesign', design=[], line=None, filename=None)
SQARequirementIssues = tokens.newToken('SQARequirementIssues', issues=[], line=None, filename=None)
SQARequirementSpecification = tokens.newToken('SQARequirementSpecification',
                                              spec_name=None, spec_path=None)
SQARequirementPrequisites = tokens.newToken('SQARequirementPrequisites', specs=[])
SQARequirementDetails = tokens.newToken('SQARequirementDetails')
SQARequirementDetailItem = tokens.newToken('SQARequirementDetailItem', label=None)

SQARequirementMatrixHeading = tokens.newToken('SQARequirementMatrixHeading', category=None)

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
        config['url'] = (u'https://github.com', "Deprecated, see 'repos'.")
        config['repo'] = (None, "Deprecated, see 'repos'.")

        config['repos'] = (dict(default=u"https://github.com/idaholab/moose"),
                           "The repository locations for linking issues, set 'default' to allow " \
                           "'#1234' or add additional keys to allow for foo#1234.")
        config['categories'] = (dict(), "A dictionary of category names that includes a " \
                                        "dictionary with 'directories' and optionally 'specs' " \
                                        "and 'dependencies'.")
        config['requirement-groups'] = (dict(), "Allows requirement group names to be changed.")

        # Disable by default to allow for updates to applications
        config['active'] = (False, config['active'][1])
        return config

    def __init__(self, *args, **kwargs):
        command.CommandExtension.__init__(self, *args, **kwargs)

        # Build requirements sets
        self.__requirements = dict()
        self.__dependencies = dict()
        for index, (category, info) in enumerate(self.get('categories').items(), 1): #pylint: disable=no-member
            specs = info.get('specs', ['tests'])
            directories = []
            for d in info.get('directories'):
                path = mooseutils.eval_path(d)
                if not os.path.isdir(path):
                    path = os.path.join(MooseDocs.ROOT_DIR, d)
                if not os.path.isdir(path):
                    msg = "Input directory does not exist: %s"
                    LOG.error(msg, path)

                directories.append(path)

            # Create requirement database
            self.__requirements[category] = common.get_requirements(directories, specs, 'F', index)

            # Create dependency database
            self.__dependencies[category] = info.get('dependencies', [])

        # Storage for requirement matrix counting (see SQARequirementMatricCommand)
        self.__counts = collections.defaultdict(int)

        # Deprecate 'url' and 'repo' config options
        url = self.get('url')
        repo = self.get('repo')
        if repo is not None:
            msg = "The 'url' and 'repo' config options for MooseDocs.extensions.sqa are deprecated,"\
                 " add the 'repos' option with a 'default' entry instead."
            LOG.warning(msg)
            self['repos'].update(dict(default="{}/{}".format(url, repo)))

        # Always include MOOSE and libMesh
        self['repos'].update(dict(moose=u"https://github.com/idaholab/moose",
                                  libmesh=u"https://github.com/libMesh/libmesh"))

    def requirements(self, category):
        """Return the requirements dictionaries."""
        req = self.__requirements.get(category, None)
        if req is None:
            raise exceptions.MooseDocsException("Unknown or missing 'category': {}", category)
        return req

    def dependencies(self, category):
        """Return the dependency list for given category."""
        dep = self.__dependencies.get(category, None)
        if dep is None:
            raise exceptions.MooseDocsException("Unknown or missing 'category': {}", category)
        return dep

    def preExecute(self, root):
        """Reset counts."""
        self.__counts.clear()

    def increment(self, key):
        """Increment and return count for requirements matrix."""
        self.__counts[key] += 1
        return self.__counts[key]

    def extend(self, reader, renderer):
        self.requires(core, command, autolink, floats)

        self.addCommand(reader, SQARequirementsCommand())
        self.addCommand(reader, SQARequirementsMatrixCommand())
        self.addCommand(reader, SQAVerificationCommand())
        self.addCommand(reader, SQACrossReferenceCommand())
        self.addCommand(reader, SQADependenciesCommand())
        self.addCommand(reader, SQADocumentCommand())

        renderer.add('SQARequirementMatrix', RenderSQARequirementMatrix())
        renderer.add('SQARequirementMatrixItem', RenderSQARequirementMatrixItem())
        renderer.add('SQARequirementMatrixListItem', RenderSQARequirementMatrixListItem())
        renderer.add('SQARequirementMatrixHeading', RenderSQARequirementMatrixHeading())
        renderer.add('SQARequirementText', RenderSQARequirementText())
        renderer.add('SQARequirementDesign', RenderSQARequirementDesign())
        renderer.add('SQARequirementIssues', RenderSQARequirementIssues())
        renderer.add('SQARequirementSpecification', RenderSQARequirementSpecification())
        renderer.add('SQARequirementPrequisites', RenderSQARequirementPrequisites())
        renderer.add('SQARequirementDetails', RenderSQARequirementDetails())
        renderer.add('SQARequirementDetailItem', RenderSQARequirementDetailItem())

        if isinstance(renderer, LatexRenderer):
            renderer.addPackage('xcolor')
            renderer.addPreamble(LATEX_REQUIREMENT)

        if isinstance(renderer, HTMLRenderer):
            renderer.addCSS('sqa_moose', "css/sqa_moose.css")

class SQARequirementsCommand(command.CommandComponent):
    COMMAND = 'sqa'
    SUBCOMMAND = 'requirements'

    @staticmethod
    def defaultSettings():
        config = command.CommandComponent.defaultSettings()
        config['category'] = (None, "Provide the category.")
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
        category = self.settings.get('category', None)
        if category == '_empty_':
            return parent

        group_map = self.extension.get('group_map', dict())
        for group, requirements in self.extension.requirements(category).items():
            group = group_map.get(group, group.replace('_', ' ').title())

            matrix = SQARequirementMatrix(parent)
            SQARequirementMatrixHeading(matrix, category=category, string=str(group))
            for req in requirements:
                self._addRequirement(matrix, info, page, req, requirements)

        return parent

    def _addRequirement(self, parent, info, page, req, requirements):
        reqname = u"{}:{}".format(req.path, req.name) if req.path != '.' else req.name
        item = SQARequirementMatrixItem(parent, label=req.label, reqname=reqname,
                                        satisfied=req.satisfied)
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

        if req.details:
            details = SQARequirementDetails(item)
            for detail in req.details:
                ditem = SQARequirementDetailItem(details)
                text = SQARequirementText(ditem)
                self.reader.tokenize(text, detail.text, page, MooseDocs.INLINE, info.line, \
                                     report=False)

        if self.settings['link']:
            if self.settings['link-spec']:
                p = SQARequirementSpecification(item, spec_path=req.path, spec_name=req.name)

                hit_root = mooseutils.hit_load(req.filename)
                h = hit_root.find(req.name)
                content = h.render()

                floats.create_modal_link(p,
                                         title=reqname, string=reqname,
                                         content=core.Code(None, language=u'text', content=content))

            if self.settings['link-design'] and req.design:
                p = SQARequirementDesign(item, filename=req.filename, design=req.design,
                                         line=req.design_line)

            if self.settings['link-issues'] and req.issues:
                p = SQARequirementIssues(item, filename=req.filename, issues=req.issues,
                                         line=req.issues_line)

            if self.settings.get('link-prerequisites', False) and req.prerequisites:
                labels = []
                for prereq in req.prerequisites:
                    for other in requirements:
                        if (other.name == prereq) and (other.path == req.path):
                            labels.append((other.path, other.name, other.label))
                        for detail in other.details:
                            if (detail.name == prereq) and (detail.path == req.path):
                                labels.append((other.path, other.name, other.label))

                SQARequirementPrequisites(item, specs=labels)


class SQACrossReferenceCommand(SQARequirementsCommand):
    COMMAND = 'sqa'
    SUBCOMMAND = 'cross-reference'

    @staticmethod
    def defaultSettings():
        config = SQARequirementsCommand.defaultSettings()
        config['category'] = (None, "Provide the category.")
        config.pop('link-prerequisites')
        return config

    def createToken(self, parent, info, page):
        design = collections.defaultdict(list)
        category = self.settings.get('category')
        if category == '_empty_':
            return parent

        for requirements in self.extension.requirements(category).values():
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

        for node, requirements in design.items():
            matrix = SQARequirementMatrix(parent)
            heading = SQARequirementMatrixHeading(matrix, category=category)
            autolink.AutoLink(heading, page=str(node.local))
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
        config['category'] = (None, "Provide the category.")
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
        config['category'] = (None, "Provide the category.")
        return config

    def createToken(self, parent, info, page):
        category = self.settings.get('category')
        if category == '_empty_':
            return parent

        matrix = SQARequirementMatrix(parent)
        for requirements in self.extension.requirements(category).values():
            for req in requirements:
                if getattr(req, info['subcommand']) is not None:
                    self._addRequirement(matrix, info, page, req)

        return parent

    def _addRequirement(self, parent, info, page, req):
        reqname = u"{}:{}".format(req.path, req.name) if req.path != '.' else req.name
        item = SQARequirementMatrixItem(parent, label=req.label, reqname=reqname)
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
                                     string=reqname,
                                     content=core.Code(None, language=u'text', content=content),
                                     title=str(req.filename))

        p = core.Paragraph(item)
        tokens.String(p, content=u'Documentation: ')
        filename = getattr(req, info['subcommand'])
        autolink.AutoLink(p, page=str(filename))

class SQADependenciesCommand(command.CommandComponent):
    COMMAND = 'sqa'
    SUBCOMMAND = 'dependencies'

    @staticmethod
    def defaultSettings():
        config = command.CommandComponent.defaultSettings()
        config['suffix'] = (None, "Provide the filename suffix to include.")
        config['category'] = (None, "Provide the category.")
        return config

    def createToken(self, parent, info, page):
        suffix = self.settings['suffix']
        category = self.settings['category']
        if category == '_empty_':
            depends = self.extension.get('categories').keys()
        else:
            depends = self.extension.dependencies(category) or \
                self.extension.get('categories').keys()

        ul = core.UnorderedList(parent)
        for dep in depends:
            if dep != category:
                fname = '{}_{}.md'.format(dep, suffix)
                autolink.AutoLink(core.ListItem(ul), page=u'sqa/{}'.format(fname),
                                  optional=True, warning=True, class_='moose-sqa-dependency')
        return parent

class SQADocumentCommand(command.CommandComponent):
    COMMAND = 'sqa'
    SUBCOMMAND = 'document'

    @staticmethod
    def defaultSettings():
        config = command.CommandComponent.defaultSettings()
        config['suffix'] = (None, "Provide the filename suffix to include.")
        config['category'] = (None, "Provide the category.")
        return config

    def createToken(self, parent, info, page):
        category = self.settings.get('category')
        suffix = self.settings.get('suffix')
        return autolink.AutoLink(parent, page=u'sqa/{}_{}.md'.format(category, suffix),
                                 optional=True, warning=True)

class RenderSQARequirementMatrix(core.RenderUnorderedList):

    def createHTML(self, parent, token, page):
        return html.Tag(parent, 'ul', class_='moose-sqa-requirements')

    def createMaterialize(self, parent, token, page):
        tag = self.createHTML(parent, token, page)
        tag.addClass('collection')
        tag.addClass('with-header')
        return tag

    def createLatex(self, parent, token, page):
        return parent

class RenderSQARequirementMatrixItem(core.RenderListItem):
    def createHTML(self, parent, token, page):
        attributes = copy.copy(token.attributes)

        li = html.Tag(parent, 'li', **attributes)
        num = html.Tag(li, 'span', string=token['label'], class_='moose-sqa-requirement-number')

        if not token['satisfied']:
            num.addClass('moose-sqa-requirement-unsatisfied')

        return html.Tag(li, 'span', class_='moose-sqa-requirement-content')

    def createMaterialize(self, parent, token, page): #pylint: disable=no-self-use,unused-argument
        attributes = copy.copy(token.attributes)
        id_ = token['reqname']

        li = html.Tag(parent, 'li', class_="collection-item", **attributes)
        num = html.Tag(li, 'span', string=token['label'], id_=id_,
                       class_='moose-sqa-requirement-number')
        num.addClass('tooltipped')
        num['data-tooltip'] = id_

        if not token['satisfied']:
            num.addClass('moose-sqa-requirement-unsatisfied')

        return html.Tag(li, 'span', class_='moose-sqa-requirement-content')

    def createLatex(self, parent, token, page):
        label = token['label']
        if not token['satisfied']:
            brace = latex.Brace()
            latex.Command(brace, 'textcolor',
                          args=[latex.Brace(string='red')],
                          string=label)
            args = [brace]
        else:
            args = [latex.Brace(string=label)]
        return latex.Environment(parent, 'Requirement', args=args)

class RenderSQARequirementMatrixListItem(RenderSQARequirementMatrixItem):

    def createHTML(self, parent, token, page):
        li = html.Tag(parent, 'li', token)
        html.Tag(li, 'span', string=token['label'], class_='moose-sqa-requirement-number')
        return html.Tag(li, 'span', class_='moose-sqa-requirement-content')

    def createMaterialize(self, parent, token, page): #pylint: disable=no-self-use,unused-argument
        li = html.Tag(parent, 'li', token, class_="collection-item")
        html.Tag(li, 'span', string=token['label'], class_='moose-sqa-requirement-number')
        return html.Tag(li, 'span', class_='moose-sqa-requirement-content')

    def createLatex(self, parent, token, page):
        args = [latex.Brace(string=token['label'])]
        return latex.Environment(parent, 'Requirement', args=args)

class RenderSQARequirementMatrixHeading(core.RenderListItem):

    def createHTML(self, parent, token, page):
        return None

    def createMaterialize(self, parent, token, page):
        tag = html.Tag(parent, 'li', class_='collection-header')
        category = token.get('category')
        if category:
            tag.insert(0, html.String(content='{}: '.format(category)))
        return tag

    def createLatex(self, parent, token, page):
        prefix = token.siblings[0]['label'].split('.')[0]
        return latex.Command(parent, 'section*', string=prefix + ':~', escape=False)

class RenderSQARequirementText(components.RenderComponent):
    def createHTML(self, parent, token, page):
        return parent

    def createLatex(self, parent, token, page):
        return parent

class RenderSQARequirementDesign(autolink.RenderLinkBase):

    def findDesign(self, filename, design, line):
        node = self.translator.findPage(design, throw_on_zero=False)
        if node is None:
            msg = "Unable to locate the design page: {}\n    {}:{}" \
                  .format(design, filename, line)
            LOG.error(msg)
            return None
        return node

    def createHTML(self, parent, token, page):
        p = html.Tag(parent, 'p', string=u'Design: ', class_='moose-sqa-items')
        for design in token['design']:
            node = self.findDesign(token['filename'], design, token['line'])
            if node is not None:
                link = autolink.AutoLink(None, page=page)
                link.info = token.info
                self.createHTMLHelper(p, link, page, node)
            else:
                html.Tag(p, 'a', string=str(design), class_='moose-sqa-error')

    def createLatex(self, parent, token, page):
        prev = token.previous
        if prev and prev.name != 'RenderSQARequirementDetails':
            latex.Command(parent, 'newline', start='\n', end='\n')

        latex.String(parent, content='Design:~', escape=False)
        no_seperator = True
        for design in token['design']:
            if not no_seperator:
                latex.String(parent, content='; ')
                no_seperator = False

            node = self.findDesign(token['filename'], design, token['line'])
            if node:
                link = autolink.AutoLink(None, page=page)
                link.info = token.info
                self.createLatexHelper(parent, link, page, node)
            else:
                latex.Command(parent, 'textcolor', args=[latex.Brace(string='red')], string=design)

class RenderSQARequirementIssues(components.RenderComponent):

    ISSUE_RE = re.compile(r"(?P<key>\w+)?#(?P<issues>[0-9]+)")
    COMMIT_RE = re.compile(r"(?:(?P<key>\w+):)?(?P<commit>[0-9a-f]{10,40})")

    def __urlHelper(self, regex, name, issue, token):
        """Function for creating issue/commit links."""
        repos = self.extension['repos']
        default = repos.get('default', None)
        match = regex.search(issue)
        if match:
            key = match.group('key')
            repo = repos.get(key, None)
            if (key is not None) and (repo is None):
                msg = "Unknown key '{}' for MooseDocs.extensions.sqa 'repos' config.\n    {}:{}"
                raise exceptions.MooseDocsException(msg, key, token['filename'], token['line'])
            repo = repo or default
            url = u"{}/{}/{}".format(repo, name, match.group(name))
            return url

    def getURL(self, issue, token):
        url = None
        url = self.__urlHelper(self.ISSUE_RE, 'issues', issue, token)
        if url is None:
            url = self.__urlHelper(self.COMMIT_RE, 'commit', issue, token)

        if (url is None) and (issue != u''):
            msg = "Unknown issue number or commit (commit SHA-1 must be at least 10 digits): "\
                  "{}\n    {}:{}".format(issue, token['filename'], token['line'])
            LOG.error(msg)

        return url

    def createHTML(self, parent, token, page):

        p = html.Tag(parent, 'p', string=u'Issue(s): ', class_='moose-sqa-items')
        for issue in token['issues']:

            url = self.getURL(issue, token)
            if url is None:
                html.Tag(p, 'a', string=issue, class_='moose-error')
            else:
                html.Tag(p, 'a', href=url, string=str(issue))

    def createLatex(self, parent, token, page):
        prev = token.previous
        if prev and prev.name != 'SQARequirementDetails':
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
                latex.Command(parent, 'href', args=[latex.Brace(string=url)], string=str(issue))

class RenderSQARequirementSpecification(components.RenderComponent):

    def createMaterialize(self, parent, token, page):
        return html.Tag(parent, 'p', string=u'Specification: ')

    def createHTML(self, parent, token, page):
        spath = token['spec_path']
        if spath == '.':
            spec = u'Specification: {}'.format(token['spec_name'])
        else:
            spec = u'Specification: {}:{}'.format(token['spec_path'], token['spec_name'])
        html.Tag(parent, 'p', string=spec)

    def createLatex(self, parent, token, page):
        prev = token.previous
        if prev and prev.name != 'SQARequirementDetails':
            latex.Command(parent, 'newline', start='\n', end='\n')
        spath = token['spec_path']
        if spath == '.':
            spec = 'Specification: {}'.format(token['spec_name'])
        else:
            spec = 'Specification: {}:{}'.format(token['spec_path'], token['spec_name'])
        latex.String(parent, content=spec)

class RenderSQARequirementPrequisites(components.RenderComponent):
    def createHTML(self, parent, token, page):
        p = html.Tag(parent, 'p', string=u'Prerequisite(s): ', class_='moose-sqa-items')

        for path, name, label in token['specs']:
            url = u'#{}:{}'.format(path, name)
            html.Tag(p, 'a', href=url, string=label)

    def createLatex(self, parent, token, page):
        prev = token.previous
        if prev and prev.name != 'SQARequirementDetails':
            latex.Command(parent, 'newline', start='\n', end='\n')
        latex.String(parent, content='Prerequisite(s):~', escape=False)

        labels = [label for _, _, label in token['specs']]
        latex.String(parent, content='; '.join(labels))

class RenderSQARequirementDetails(components.RenderComponent):
    def createHTML(self, parent, token, page):
        return html.Tag(parent, 'ol', class_='moose-sqa-details-list')

    def createLatex(self, parent, token, page):
        return latex.Environment(parent, 'enumerate', after_begin='', after_end='', info=token.info)

class RenderSQARequirementDetailItem(components.RenderComponent):
    def createHTML(self, parent, token, page):
        return html.Tag(parent, 'li', class_='moose-sqa-detail-item')

    def createLatex(self, parent, token, page):
        latex.Command(parent, 'item', start='\n', end=' ')
        return parent
