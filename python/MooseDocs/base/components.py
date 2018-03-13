"""
An Extension is comprised of Component objects, the objects are used for tokenizeing markdown
and converting tokens to rendered HTML.
"""
from MooseDocs.common import exceptions, parse_settings, mixins

class Extension(mixins.ConfigObject, mixins.TranslatorObject):
    """
    Base class for creating extensions. An extension is simply a mechanism to allow for
    the creation of reader/renderer components to be added to the translation process.

    All aspects of the MooseDocs system rely on Extension objects. These extensions are passed
    to the Translator object. The translator calls the extend method of the extension.

    Inputs:
        kwargs: All key-value pairs are treated as configure options, see ConfigObject.

    The Translator object allows for pre/post calls to the Extension object for tokenization
    and rendering. Within your extension one of the following methods exist it will be called
    automatically.

        preTokenize(ast, config)
        postTokenize(ast, config)
        preRender(root, config)
        postRender(root, config)
    """
    def __init__(self, **kwargs):
        mixins.ConfigObject.__init__(self, **kwargs)
        mixins.TranslatorObject.__init__(self)

    def extend(self, reader, renderer):
        """
        Method for adding reader and renderering components.
        """
        pass

    def requires(self, *args):
        """
        Require that the supplied extension module exists within the Translator object. This
        method cannot be called before init().
        """
        if self.translator is None:
            raise exceptions.MooseDocsException("The 'requires' method should be called from " \
                                                "within the 'extend' method.")

        available = [e.__module__ for e in self.translator.extensions]
        messages = []
        for ext in args:
            if ext.__name__ not in available:
                msg = "The {} extension is required but not included.".format(ext.__name__)
                messages.append(msg)

        if messages:
            raise exceptions.MooseDocsException('\n'.join(messages))


class Component(mixins.TranslatorObject):
    """
    Each extension is made up of components, both for tokenizing and rendering. The components
    provide a means for defining settings as well as other customizable features required for
    translation.
    """
    def __init__(self):
        mixins.TranslatorObject.__init__(self)
        self.extension = None

class TokenComponent(Component):
    """
    Base class for creating components designed to create a token during tokenization.

    # Overview
    TokenComponent objects are designed to be created by Extension objects and added to the Reader
    object in the Extension:extend method. The purpose of a TokenComponent is to define a regular
    expression, that when match returns a token that is added to the AST.

    The codebase of MooseDocs (Translator, Reader, Renderer) is certainly capable of tokenizing
    arbitrary formats. However, the primary use case is for creating HTML so there are a certain
    features for this class that are setup with that application in mind.

    ## RE Class Member
    The RE class member variable is used for convenience to
    allow for derived classes to avoid creating an __init__ method. As shown in core.py the purpose
    of this class is to be able to add parser syntax for tokenizing with a minimal amount code, the
    following static member variables aid in the design goal.

    RE must be defined as a compile re expressions that captures the content to be converted to a
    token within the createToken method.  - TOKEN: The token type (not instance) that will be
    created

    ## Settings
    If the supplied regex match option has a group named "settings" these settings will be
    automatically parsed by the translator and used to update the values returned by the
    defaultSettings method. These settings must be key-value pairings that use an equal sign that is
    does not contains spaces on either side (e.g., foo=bar).

    The automatic parsing settings may be disable by setting the PARSE_SETTINGS class member
    variable to False in the child object.
    """
    RE = None
    PARSE_SETTINGS = True

    @staticmethod
    def defaultSettings():
        """
        Default settings for the component. Child classes should define a similar method to create
        the default settings for the component, see core.py for examples.
        """
        settings = dict()
        settings['style'] = (u'', u"The style settings that are passed to rendered HTML tag.")
        settings['class'] = (u'', u"The class settings to be passed to rendered HTML tag.")
        settings['id'] = (u'', u"The class settings to be passed to the rendered tag.")
        return settings

    def __init__(self):
        """
        Constructs the object and sets the default settings of the object.
        """
        Component.__init__(self)

        # Local settings, this is updated by __call__ just prior to calling the createToken()
        self.__settings = None

        # Check return type of default settings
        defaults = self.defaultSettings()
        if not isinstance(defaults, dict):
            msg = "The component '{}' must return a dict from the defaultSettings static method."
            raise exceptions.MooseDocsException(msg, self)

    def __call__(self, info, parent):
        """
        MooseDocs internal method, this should not be called, please use the createToken method.

        The lexer system within MooseDocs expects a function this method allows this class to act
        as a function.

        Inputs:
            info[LexerInformation]: Object containing the lexer information object.
            parent[tokens.Token]: The parent node in the AST for the token being created.
        """

        # Define the settings
        defaults = self.defaultSettings()
        if self.PARSE_SETTINGS and ('settings' in info):
            self.__settings, _ = parse_settings(defaults, info['settings'])
        else:
            self.__settings = {k:v[0] for k, v in defaults.iteritems()}

        # Call user method and reset settings
        token = self.createToken(info, parent)
        self.__settings = None
        return token

    @property
    def attributes(self):
        """
        Return a dictionary with the common html settings.

        This property is only available from within the createToken method, it returns None when
        called externally.
        """
        if self.__settings:
            return {'style':self.settings['style'].strip(),
                    'id':self.settings['id'].strip(),
                    'class':self.settings['class'].strip()}
        return dict()

    @property
    def settings(self):
        """
        Retrun a copy of the settings, without the setting descriptions.
        """
        return self.__settings

    @property
    def reader(self):
        """
        Return the Reader object.
        """
        return self.translator.reader

    def setSettings(self, settings):
        """
        Method for defining the settings for this object directly.

        This is required to allow for the command extension to work correctly.
        """
        self.__settings = settings

    def createToken(self, info, parent):
        """
        Method designed to be implemented by child classes, this method should create the
        token for the AST based on the regex match.

        Inputs:
            info[LexerInformation]: Object containing the lexer information object.
            parent[tokens.Token]: The parent node in the AST for the token being created.
        """
        raise NotImplementedError("The createToken method is required.")

class RenderComponent(Component):
    """
    RenderComponent objects are used to convert tokens to an output format such as HTML or LaTeX.

    The function to be called is assigned by the Renderer object; however, it has the following
    signature:
        functionName(self, token, parent):
            ...

    This allows the RenderComponent object to have multiple methods for converting to different
    formats. For example, the components in core.py have createHTML and createLatex methods to
    work with the HTMLRenderer and the LatexRenderer.
    """

    @property
    def renderer(self):
        """
        Return the Renderer object.
        """
        return self.translator.renderer
