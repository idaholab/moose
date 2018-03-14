#pylint: disable=missing-docstring
import re
import os
import collections
import codecs

import anytree

import mooseutils

import MooseDocs
from MooseDocs import common
from MooseDocs.common import exceptions
from MooseDocs.base import components
from MooseDocs.extensions import command, alert, floats, core, autolink
from MooseDocs.tree import tokens, html

def make_extension(**kwargs):
    return SQAExtension(**kwargs)

#: tuple for storing "requirement" from test specification
Requirement = collections.namedtuple('Requirement', "name path filename requirement design issues")

def _get_requirements():
    """

    Helper for examining test specification for "requirements."

    TODO: This should be updated to accept an arbitrary directories and re-factored a bit to avoid
          the six levels of nesting.
    """

    directories = [os.path.join(MooseDocs.ROOT_DIR, 'test', 'tests')]
    spec = 'tests'
    out = set()
    for location in directories:
        for base, _, files in os.walk(location):
            for fname in files:
                if fname == spec:
                    full_file = os.path.join(base, fname)
                    root = mooseutils.hit_load(full_file)
                    for child in root.children[0]:
                        #TODO: Make 'issues' optional?
                        if ('requirement' in child) and ('design' in child) and ('issues' in child):
                            req = Requirement(name=child.name,
                                              path=os.path.relpath(base, location),
                                              filename=full_file,
                                              requirement=child['requirement'],
                                              design=child['design'],
                                              issues=child['issues'])
                            out.add(req)
    return out

class SQAExtension(command.CommandExtension):

    @staticmethod
    def defaultConfig():
        config = command.CommandExtension.defaultConfig()
        return config

    def __init__(self, *args, **kwargs):
        command.CommandExtension.__init__(self, *args, **kwargs)

         #TODO: re-compute on reinit and move to the command object
        self.requirements = _get_requirements()


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
                  tokens.Property('use_default', ptype=bool, required=True)]
class SQARequirementMatrix(tokens.OrderedList):
    pass

class SQARequirementMatrixItem(tokens.ListItem):
    pass

class SQARequirementsCommand(command.CommandComponent):
    COMMAND = 'sqa'
    SUBCOMMAND = 'requirements'

    @staticmethod
    def defaultSettings():
        config = command.CommandComponent.defaultSettings()
        config['link_tests'] = (True, "Enable/disable the linking of test specifications and " \
                                      "test files.")
        return config

    def createToken(self, info, parent):
        matrix = SQARequirementMatrix(parent)
        for req in self.extension.requirements:
            item = SQARequirementMatrixItem(matrix)
            self.translator.reader.parse(item, unicode(req.requirement))

            #TODO: Make option
            p = tokens.Paragraph(item, 'p')
            tokens.String(p, content=u'Specification: ')


            with codecs.open(req.filename, encoding='utf-8') as fid:
                content = fid.read()

            floats.ModalLink(p, 'a', tooltip=False, url=u"#",
                             string=u"{}:{}".format(req.path, req.name),
                             title=tokens.String(None, content=unicode(req.filename)),
                             content=tokens.Code(None, language=u'text', code=content))

            #TODO: Make option
            if req.design:
                p = tokens.Paragraph(item, 'p')
                tokens.String(p, content=u'Design: ')
                for design in req.design.split():
                    autolink.AutoShortcutLink(p, key=unicode(design))

            #TODO: Make option
            if req.issues:
                p = tokens.Paragraph(item, 'p')
                tokens.String(p, content=u'Issues: ')
                for issue in req.issues.split():
                    url = u"https://github.com/idaholab/moose/issues/{}".format(issue[1:])
                    tokens.Link(p, url=url, string=unicode(issue))

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
        config['use_default'] = (False, "Dislpay the default if template item is not provided in " \
                                        "the parent document.")
        return config

    def createToken(self, info, parent):
        return SQATemplateItem(parent,
                               key=self.settings['key'],
                               use_default=self.settings['use_default'])

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

        #def func(node):
        func = lambda n: isinstance(n, SQADocumentItem) and (n.key == key)
        replacement = anytree.search.find(token.root, filter_=func, maxlevel=2)

        if replacement:
            self.translator.renderer.process(parent, replacement)

        elif token.use_default:
            for child in token.children:
                self.translator.renderer.process(parent, child)

        else:
            err = alert.AlertToken(token.parent, brand='error',
                                   title=u'Missing Template Item "{}"'.format(key))

            filename = token.root.page.source
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
        return html.Tag(parent, 'ol', class_="collection")

class RenderSQARequirementMatrixItem(core.RenderListItem):
    def createMaterialize(self, token, parent): #pylint: disable=no-self-use,unused-argument
        return html.Tag(parent, 'li', class_="collection-item")
