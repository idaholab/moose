import os
import markdown
import collections
import cPickle as pickle
import logging
log = logging.getLogger(__name__)

from MooseObjectSyntax import MooseObjectSyntax
from MooseParameters import MooseParameters
from MooseDescription import MooseDescription
from MooseActionSyntax import MooseActionSyntax
from MooseTextFile import MooseTextFile
from MooseImageFile import MooseImageFile
from MooseFigure import MooseFigure
from MooseFigureReference import MooseFigureReference
from MooseEquationReference import MooseEquationReference
from MooseInlineProcessor import MooseInlineProcessor
from MooseInputBlock import MooseInputBlock
from MooseCppMethod import MooseCppMethod
from MoosePackageParser import MoosePackageParser
from MooseSlider import MooseSlider
from MooseDiagram import MooseDiagram
from MooseCSS import MooseCSS
from MooseCSSPreprocessor import MooseCSSPreprocessor
from MooseBuildStatus import MooseBuildStatus
from MooseBibtex import MooseBibtex
from MooseActionList import MooseActionList
from MooseCopyCodeButton import MooseCopyCodeButton
from MooseContentScroll import MooseContentScroll
from MooseTemplate import MooseTemplate
import MooseDocs
import mooseutils

class MooseMarkdownExtension(markdown.Extension):
    """
    Extensions that comprise the MOOSE flavored markdown.
    """

    def __init__(self, **kwargs):

        # Storage for the MooseLinkDatabase object
        self.syntax = None

        # Define the configuration options
        self.config = dict()
        self.config['executable']    = ['', "The executable to utilize for generating application syntax."]
        self.config['locations']     = [dict(), "The locations to parse for syntax."]
        self.config['repo']          = ['', "The remote repository to create hyperlinks."]
        self.config['links']         = [dict(), "The set of paths for generating input file and source code links to objects."]
        self.config['package']       = [False, "Enable the use of the MoosePackageParser."]
        self.config['graphviz']      = ['/opt/moose/graphviz/bin', 'The location of graphviz executable for use with diagrams.']
        self.config['dot_ext']       = ['svg', "The graphviz/dot output file extension (default: svg)."]
        self.config['install']       = ['', "The location to install system and object documentation."]
        self.config['macro_files']   = ['', "List of paths to files that contain macros to be used in bibtex parsing."]
        self.config['template']      = ['', "The jinja2 template to apply."]
        self.config['template_args'] = [dict(), "Arguments passed to to the MooseTemplate Postprocessor."]

        # Construct the extension object
        super(MooseMarkdownExtension, self).__init__(**kwargs)

        # Create the absolute path to the executable
        self.setConfig('executable', MooseDocs.abspath(self.getConfig('executable')))

    def execute(self):
        """
        Execute the supplied MOOSE application and return the YAML.
        """

        cache = os.path.join(MooseDocs.TEMP_DIR, 'moosedocs.yaml')
        exe = self.getConfig('executable')

        if os.path.exists(cache) and (os.path.getmtime(cache) >= os.path.getmtime(cache)):
            with open(cache, 'r') as fid:
                log.debug('Reading MooseYaml Pickle: ' + cache)
                return mooseutils.MooseYaml(pickle.load(fid))

        elif not (exe or os.path.exists(exe)):
            log.critical('The executable does not exist: {}'.format(exe))
            raise Exception('Critical Error')

        else:
            log.debug("Executing {} to extract syntax.".format(exe))
            try:
                raw = mooseutils.runExe(exe, '--yaml')
                with open(cache, 'w') as fid:
                    log.debug('Writing MooseYaml Pickle: ' + cache)
                    pickle.dump(raw, fid)
                return mooseutils.MooseYaml(raw)
            except:
                log.critical('Failed to read YAML file, MOOSE and modules are likely not compiled correctly.')
                raise Exception('Critical Error')


    def extendMarkdown(self, md, md_globals):
        """
        Builds the extensions for MOOSE flavored markdown.
        """
        md.registerExtension(self)

        # Create a config object
        config = self.getConfigs()

        # Default template arguments
        config['template_args'].setdefault('title', None)
        config['template_args'].setdefault('logo', None)
        config['template_args'].setdefault('repo_url', None)
        config['template_args'].setdefault('navigation', 'navigation.yml')

        # Extract YAML
        exe_yaml = self.execute()

        # Generate YAML data from application
        # Populate the database for input file and children objects
        log.debug('Creating input file and source code use database.')
        database = MooseDocs.MooseLinkDatabase(**config)

        # Populate the syntax
        self.syntax = collections.OrderedDict()
        for item in config['locations']:
            key = item.keys()[0]
            options = item.values()[0]
            options.setdefault('group', key)
            options.setdefault('name', key.replace('_', ' ').title())
            options.setdefault('install', config['install'])
            self.syntax[key] = MooseDocs.MooseApplicationSyntax(exe_yaml, **options)

        # Replace the InlineTreeprocessor with the MooseInlineProcessor, this allows
        # for an initialize() method to be called prior to the convert for re-setting state.
        md.treeprocessors['inline'] = MooseInlineProcessor(markdown_instance=md, **config)

        # Preprocessors
        md.preprocessors.add('moose_bibtex', MooseBibtex(markdown_instance=md, **config), '_end')
        md.preprocessors.add('moose_css_list', MooseCSSPreprocessor(markdown_instance=md, **config), '_end')

        # Block processors
        md.parser.blockprocessors.add('diagrams', MooseDiagram(md.parser, **config), '_begin')
        md.parser.blockprocessors.add('slider', MooseSlider(md.parser, **config), '_begin')
        md.parser.blockprocessors.add('css', MooseCSS(md.parser, **config), '_begin')

        # Inline Patterns
        params = MooseParameters(markdown_instance=md, syntax=self.syntax, **config)
        md.inlinePatterns.add('moose_parameters', params, '_begin')

        desc = MooseDescription(markdown_instance=md, syntax=self.syntax, **config)
        md.inlinePatterns.add('moose_description', desc, '_begin')

        object_markdown = MooseObjectSyntax(markdown_instance=md, syntax=self.syntax, database=database, **config)
        md.inlinePatterns.add('moose_object_syntax', object_markdown, '_begin')

        system_markdown = MooseActionSyntax(markdown_instance=md, syntax=self.syntax, **config)
        md.inlinePatterns.add('moose_system_syntax', system_markdown, '_begin')

        system_list = MooseActionList(markdown_instance=md, yaml=exe_yaml, syntax=self.syntax, **config)
        md.inlinePatterns.add('moose_system_list', system_list, '_begin')

        md.inlinePatterns.add('moose_input_block', MooseInputBlock(markdown_instance=md, **config), '_begin')
        md.inlinePatterns.add('moose_cpp_method', MooseCppMethod(markdown_instance=md, **config), '_begin')
        md.inlinePatterns.add('moose_text', MooseTextFile(markdown_instance=md, **config), '_begin')
        md.inlinePatterns.add('moose_image', MooseImageFile(markdown_instance=md, **config), '_begin')
        md.inlinePatterns.add('moose_figure', MooseFigure(markdown_instance=md, **config), '_begin')
        md.inlinePatterns.add('moose_figure_reference', MooseFigureReference(markdown_instance=md, **config), '>moose_figure')
        md.inlinePatterns.add('moose_equation_reference', MooseEquationReference(markdown_instance=md, **config), '<moose_figure_reference')
        md.inlinePatterns.add('moose_build_status', MooseBuildStatus(markdown_instance=md, **config), '_begin')
        if config['package']:
            md.inlinePatterns.add('moose_package_parser', MoosePackageParser(markdown_instance=md, **config), '_end')

        # Postprocessing
        md.treeprocessors.add('moose_code_button', MooseCopyCodeButton(markdown_instance=md, **config), '_end')
        md.treeprocessors.add('moose_content_scroll', MooseContentScroll(markdown_instance=md, **config), '_end')

        if config['template']:
            md.postprocessors.add('moose_template', MooseTemplate(markdown_instance=md, **config), '_end')


def makeExtension(*args, **kwargs):
    return MooseMarkdownExtension(*args, **kwargs)
