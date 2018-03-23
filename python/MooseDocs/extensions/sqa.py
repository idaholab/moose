#pylint: disable=missing-docstring
import re
import os
import codecs
import logging

import anytree

import MooseDocs
from MooseDocs import common
from MooseDocs.common import exceptions
from MooseDocs.base import components
from MooseDocs.extensions import command, alert, floats, core, autolink
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
        return config

    def __init__(self, *args, **kwargs):
        command.CommandExtension.__init__(self, *args, **kwargs)

        #NOTE: This is too slow to perform on reinit()
        self.__requirements = common.get_requirements(self.get('directories'), self.get('specs'))

    @property
    def requirements(self):
        """Return the requirements dictionary."""
        return self.__requirements

    def extend(self, reader, renderer):
        self.requires(command, alert, floats, core)

        self.addCommand(SQATemplateLoadCommand())
        self.addCommand(SQATemplateItemCommand())
        self.addCommand(SQARequirementsCommand())
        self.addCommand(SQADocumentItemCommand())

        renderer.add(SQATemplateItem, RenderSQATemplateItem())
        renderer.add(SQARequirementMatrix, RenderSQARequirementMatrix())
        renderer.add(SQARequirementMatrixItem, RenderSQARequirementMatrixItem())

class SQADocumentItem(tokens.Token):
    PROPERTIES = [tokens.Property('key', ptype=unicode, required=True)]

class SQATemplateItem(tokens.Token):
    PROPERTIES = [tokens.Property('key', ptype=unicode, required=True),
                  tokens.Property('heading', ptype=tokens.Token)]

class SQARequirementMatrix(tokens.OrderedList):
    PROPERTIES = [tokens.Property('heading', ptype=unicode),
                  tokens.Property('prefix', ptype=unicode, required=True)]

class SQARequirementMatrixItem(tokens.ListItem):
    PROPERTIES = [tokens.Property('number', ptype=int, default=1)]

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

        return config

    def createToken(self, info, parent):

        number = 0
        for group, requirements in self.extension.requirements.iteritems():
            number += 1
            matrix = SQARequirementMatrix(parent,
                                          heading=unicode(group),
                                          prefix=u'F{}'.format(number))
            for i, req in enumerate(requirements):
                self._addRequirement(matrix, req, i+1)

        return parent

    def _addRequirement(self, parent, req, number):
        item = SQARequirementMatrixItem(parent, number=number, id_=req.path)
        self.translator.reader.parse(item, unicode(req.requirement))

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
                for design in req.design.split():
                    autolink.AutoShortcutLink(p, key=unicode(design))

            if self.settings['link-issues'] and req.issues:
                p = tokens.Paragraph(item, 'p')
                tokens.String(p, content=u'Issues: ')
                for issue in req.issues.split():
                    url = u"https://github.com/idaholab/moose/issues/{}".format(issue[1:])
                    tokens.Link(p, url=url, string=unicode(issue))


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
        location = os.path.join(MooseDocs.ROOT_DIR, 'framework', 'doc', 'templates', 'sqa',
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
            content = tokens.String(None, content=u'Missing Template Item "{}"'.format(key))
            err = alert.AlertToken(token.parent, brand=u'error', title=content)

            filename = self.translator.current.local
            self.translator.reader.parse(err, ERROR_CONTENT.format(key, filename))

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
            html.Tag(collection, 'li', string=token.heading, class_='collection-header')
            return collection
        else:
            return html.Tag(parent, 'ul', class_="collection")

class RenderSQARequirementMatrixItem(core.RenderListItem):
    def createMaterialize(self, token, parent): #pylint: disable=no-self-use,unused-argument
        li = html.Tag(parent, 'li', class_="collection-item", **token.attributes)
        num = html.Tag(li, 'span', string=u'{}.{}'.format(token.parent.prefix, token.number),
                       class_='moose-requirement-number')
        id_ = token.get('id', None)
        if id_:
            num.addClass('tooltipped')
            num['data-tooltip'] = id_
        return html.Tag(li, 'span', class_='moose-requirement-content')
