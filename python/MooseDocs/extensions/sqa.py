#pylint: disable=missing-docstring
import re
import os
import codecs
import logging
import collections

import anytree

import MooseDocs
from MooseDocs import common
from MooseDocs.common import exceptions
from MooseDocs.base import components
from MooseDocs.extensions import command, alert, floats, core, autolink, materialicon
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

    def reinit(self):
        """Reset counts."""
        self.__counts.clear()

    def increment(self, key):
        """Increment and return count for requirements matrix."""
        self.__counts[key] += 1
        return self.__counts[key]

    def extend(self, reader, renderer):
        self.requires(command, alert, floats, core, materialicon)

        self.addCommand(SQATemplateLoadCommand())
        self.addCommand(SQATemplateItemCommand())
        self.addCommand(SQARequirementsCommand())
        self.addCommand(SQADocumentItemCommand())
        self.addCommand(SQACrossReferenceCommand())
        self.addCommand(SQARequirementsMatrixCommand())
        self.addCommand(SQAVerificationCommand())

        renderer.add(SQATemplateItem, RenderSQATemplateItem())
        renderer.add(SQARequirementMatrix, RenderSQARequirementMatrix())
        renderer.add(SQARequirementMatrixItem, RenderSQARequirementMatrixItem())

class SQADocumentItem(tokens.Token):
    PROPERTIES = [tokens.Property('key', ptype=unicode, required=True)]

class SQATemplateItem(tokens.Token):
    PROPERTIES = [tokens.Property('key', ptype=unicode, required=True),
                  tokens.Property('heading', ptype=tokens.Token)]

class SQARequirementMatrix(tokens.OrderedList):
    PROPERTIES = [tokens.Property('heading', ptype=tokens.Token)]

class SQAVandVMatrix(SQARequirementMatrix):
    pass

class SQARequirementMatrixItem(tokens.ListItem):
    PROPERTIES = [tokens.Property('label', ptype=unicode, required=True),
                  tokens.Property('satisfied', ptype=bool, default=True)]

class SQAVandVMatrixItem(SQARequirementMatrixItem):
    pass

class SQARequirementCrossReference(tokens.Token):
    pass

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

    def createToken(self, info, parent):
        group_map = self.extension.get('requirement-groups')
        for group, requirements in self.extension.requirements.iteritems():
            group = group_map.get(group, group.replace('_', ' ').title())
            matrix = SQARequirementMatrix(parent,
                                          heading=tokens.String(None, content=unicode(group)))
            for req in requirements:
                self._addRequirement(matrix, req, requirements)

        return parent

    def _addRequirement(self, parent, req, requirements):
        item = SQARequirementMatrixItem(parent,
                                        label=unicode(req.label),
                                        satisfied=req.satisfied,
                                        id_=req.path)
        self.translator.reader.parse(item, req.text)

        if self.settings['link']:
            if self.settings['link-spec']:
                p = tokens.Paragraph(item, 'p')
                tokens.String(p, content=u'Specification: ')

                with codecs.open(req.filename, encoding='utf-8') as fid:
                    content = fid.read()

                floats.ModalLink(p, 'a', tooltip=False, url=u"#",
                                 string=u"{}:{}".format(req.path, req.name),
                                 title=tokens.String(None, content=unicode(req.filename)),
                                 content=tokens.Code(None, language=u'text', code=content))

            if self.settings['link-design'] and req.design:
                p = tokens.Paragraph(item, 'p')
                tokens.String(p, content=u'Design: ')
                for design in req.design:
                    autolink.AutoShortcutLink(p, key=unicode(design))

            if self.settings['link-issues'] and req.issues:
                p = tokens.Paragraph(item, 'p')
                tokens.String(p, content=u'Issues: ')
                for issue in req.issues:
                    url = u"https://github.com/idaholab/moose/issues/{}".format(issue[1:])
                    tokens.Link(p, url=url, string=unicode(issue))
                    tokens.Space(p)

            if self.settings['link-prerequisites'] and req.prerequisites:
                labels = []
                for prereq in req.prerequisites:
                    for other in requirements:
                        if other.name == prereq:
                            labels.append(other.label)

                p = tokens.Paragraph(item, 'p')
                tokens.String(p, content=u'Prerequisites: {}'.format(' '.join(labels)))

class SQACrossReferenceCommand(SQARequirementsCommand):
    COMMAND = 'sqa'
    SUBCOMMAND = 'cross-reference'

    def createToken(self, info, parent):
        design = collections.defaultdict(list)
        for requirements in self.extension.requirements.itervalues():
            for req in requirements:
                for d in req.design:
                    node = self.translator.current.findall(d)[0]
                    design[node].append(req)

        for node, requirements in design.iteritems():
            link = autolink.AutoShortcutLink(None, key=unicode(node.fullpath))
            link.info = info
            matrix = SQARequirementMatrix(parent, heading=link)

            for req in requirements:
                self._addRequirement(matrix, req, requirements)

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

    def createToken(self, info, parent):
        content = info['block'] if 'block' in info else info['inline']

        if self.settings['prefix'] is None:
            msg = "The 'prefix' option is required."
            raise exceptions.TokenizeException(msg)

        # Extract the unordered list
        self.reader.parse(parent, content, MooseDocs.BLOCK)
        ul = parent.children[-1]
        ul.parent = None

        # Check the list type
        if not isinstance(ul, tokens.UnorderedList):
            msg = "The content is required to be an unordered list (i.e., use '-')."
            raise exceptions.TokenizeException(msg)

        # Build the matrix
        prefix = self.settings['prefix']
        label = u'{}{:d}'.format(prefix, self.extension.increment(prefix))
        matrix = SQARequirementMatrix(parent)

        heading = self.settings['heading']
        if heading is not None:
            matrix.heading = tokens.Token(None) #pylint: disable=attribute-defined-outside-init
            self.reader.parse(matrix.heading, heading, MooseDocs.INLINE)

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

    def createToken(self, info, parent):

        #TODO: make root path a config item in extension
        location = os.path.join(MooseDocs.MOOSE_DIR, 'framework', 'doc', 'templates', 'sqa',
                                self.settings['template'])

        if not os.path.exists(location):
            msg = "The template file does not exist: {}."
            raise exceptions.TokenizeException(msg, location)

        with codecs.open(location, 'r', encoding='utf-8') as fid:
            content = fid.read()


        # Replace key/value arguments
        template_args = info['inline'] if 'inline' in info else info['block']
        _, key_values = common.match_settings(dict(), template_args)

        def sub(match):
            key = match.group('key')
            if key not in key_values:
                msg = "The template argument '{}' was not defined in the !sqa load command."
                raise exceptions.TokenizeException(msg, key)

            return key_values[key]

        content = re.sub(r'{{(?P<key>.*?)}}', sub, content)

        # Tokenize the template
        self.translator.reader.parse(parent, content)

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

    def createToken(self, info, parent):
        key = self.settings['key']
        item = SQATemplateItem(parent, key=key)

        heading = self.settings.get('heading', None)
        if heading:
            item.heading = tokens.Heading(None, #pylint: disable=attribute-defined-outside-init
                                          level=int(self.settings['heading-level']), id_=key)
            self.translator.reader.parse(item.heading, heading)

        return item

class SQADocumentItemCommand(command.CommandComponent):
    COMMAND = 'sqa'
    SUBCOMMAND = 'item'

    @staticmethod
    def defaultSettings():
        config = command.CommandComponent.defaultSettings()
        config['key'] = (None, "The name of the template item which the content is to replace.")
        return config

    def createToken(self, info, parent):
        return SQADocumentItem(parent, key=self.settings['key'])

class SQAVerificationCommand(command.CommandComponent):
    COMMAND = 'sqa'
    SUBCOMMAND = ('verification', 'validation')

    @staticmethod
    def defaultSettings():
        config = command.CommandComponent.defaultSettings()
        return config

    def createToken(self, info, parent):

        matrix = SQARequirementMatrix(parent)
        for requirements in self.extension.requirements.itervalues():
            for req in requirements:
                if getattr(req, info['subcommand']):
                    self._addRequirement(matrix, req)

        return parent

    def _addRequirement(self, parent, req):
        item = SQARequirementMatrixItem(parent,
                                        label=unicode(req.label),
                                        satisfied=req.satisfied,
                                        id_=req.path)
        self.translator.reader.parse(item, req.text)

        p = tokens.Paragraph(item, 'p')
        tokens.String(p, content=u'Specification: ')

        with codecs.open(req.filename, encoding='utf-8') as fid:
            content = fid.read()
            floats.ModalLink(p, 'a', tooltip=False, url=u"#",
                             string=u"{}:{}".format(req.path, req.name),
                             title=tokens.String(None, content=unicode(req.filename)),
                             content=tokens.Code(None, language=u'text', code=content))

        p = tokens.Paragraph(item, 'p')
        tokens.String(p, content=u'Details: ')
        filename = u'{}/{}.md'.format(req.path, req.name)
        autolink.AutoShortcutLink(p, key=unicode(filename))

class RenderSQATemplateItem(components.RenderComponent):

    def createHTML(self, token, parent):
        pass

    def createMaterialize(self, token, parent):

        key = token.key
        func = lambda n: isinstance(n, SQADocumentItem) and (n.key == key)
        replacement = anytree.search.find(token.root, filter_=func, maxlevel=2)

        if replacement:

            if token.heading is not None:
                self.translator.renderer.process(parent, token.heading)

            self.translator.renderer.process(parent, replacement)

            # Remove item so it doesn't render again
            replacement.parent = None
            for child in replacement:
                child.parent = None
#
        else:
            filename = self.translator.current.local

            content = tokens.Token(None)
            self.translator.reader.parse(content, ERROR_CONTENT.format(key, filename))

            modal_title = tokens.String(None, content=u'Missing Template Item "{}"'.format(key))

            alert_title = tokens.Token(None)
            tokens.String(alert_title, content=u'Missing Template Item "{}"'.format(key))
            h_token = floats.ModalLink(alert_title, url=unicode(filename), content=content,
                                       title=modal_title, class_='moose-help')
            materialicon.IconToken(h_token, icon=u'help_outline')

            err = alert.AlertToken(token.parent, brand=u'error', title=alert_title)
            for child in token.children:
                child.parent = err

            self.translator.renderer.process(parent, err)

ERROR_CONTENT = u"""
The document must include the \"{0}\" template item, this can be included by add adding the
following to the markdown file ({1}):

```
!sqa! item key={0}
Include text (in MooseDocs format) regarding the "{0}"
template item here.
!sqa-end!
```"""

class RenderSQARequirementMatrix(core.RenderUnorderedList):
    def createMaterialize(self, token, parent):
        if token.heading:
            collection = html.Tag(parent, 'ul', class_="moose-requirements collection with-header")
            h = html.Tag(collection, 'li', class_='collection-header')
            self.translator.renderer.process(h, token.heading)
            return collection
        else:
            return html.Tag(parent, 'ul', class_="moose-requirements collection")

class RenderSQARequirementMatrixItem(core.RenderListItem):
    def createMaterialize(self, token, parent): #pylint: disable=no-self-use,unused-argument
        li = html.Tag(parent, 'li', class_="collection-item", **token.attributes)
        num = html.Tag(li, 'span', string=token.label, class_='moose-requirement-number')

        id_ = token.get('id', None)
        if id_:
            num.addClass('tooltipped')
            num['data-tooltip'] = id_

        if not token.satisfied:
            num = html.Tag(li, 'i', string=u'block',
                           class_='material-icons moose-requirement-unsatisfied')

        return html.Tag(li, 'span', class_='moose-requirement-content')
