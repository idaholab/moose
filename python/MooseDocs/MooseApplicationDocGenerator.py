import os
import shutil
import utils
import logging
import MooseDocs
log = logging.getLogger(__name__)

from MooseSubApplicationDocGenerator import MooseSubApplicationDocGenerator
class MooseApplicationDocGenerator(object):
    """
    Reads the main configuration yml file (e.g., docs/mkdocs.yml) and generates MOOSE system and object
    documentation.

    Args:
        root[str]: The root directory of your application.
        config_file[str]: The main documentation configuration for your application.
        exe[str]: The executable to utilize.

    Kwargs:
        develop[bool]: When True the __call__ method will always generate documentation regardless of
                       modified time of the executable.
    """

    def __init__(self, config_file, **kwargs):

        self._root = os.path.dirname(config_file)
        self._config_file = config_file
        self._exe = None
        self._modified = None
        self._develop = kwargs.get('develop', False)

    def generate(self):
        """
        Operator(). Calling this function causes the documentation to generated.

        NOTE: Documentation will only generated if the executable has been modified since that last time
              the function has been called, unless the develop flag was set to True upon construction
              of this object.

        TODO: Tie this into the mkdocs livereload. To do this the mkdocs watching should be paused while
        the files are generated and then mkdocs livereload restarted. Otherwise, each file that changes
        spawns a rebuild.
        """
        if self._develop or self._exe == None:
            self._generate()
            self._modified = os.path.getmtime(self._exe)

        else:
            modified = os.path.getmtime(self._exe)
            if self._modified != os.path.getmtime(self._exe):
                self._generate()
                self._modified = modified

    def _configure(self):
        """
        Build the configuration options for included sub-directories.
        """

        # Read the moosedocs yml configuration file
        yml = MooseDocs.yaml_load(self._config_file)

        # Global defaults
        hide = yml.get('hide', list())
        install = yml.get('install', os.path.join(os.getcwd(), 'content'))
        repo = yml.get('repo', None)

        def update_config(config):
            """
            Helper for updating/creating local configuration dict.
            """

            # Defaults
            config.setdefault('details', os.path.join(os.getcwd(), 'details'))
            config.setdefault('source', None)
            config.setdefault('install', install)
            config.setdefault('prefix', '')
            config.setdefault('repo', repo)
            config.setdefault('doxygen', None)
            config.setdefault('hide', list())
            config.setdefault('links', dict())

            # Append the hide data
            config['hide'] = set(config['hide'] + hide)

            return config

        # Extract executable
        if 'app' not in yml:
            self._exe = utils.find_moose_executable(self._root)
        else:
            app = yml['app']
            if os.path.isdir(app):
                self._exe = utils.find_moose_executable(app)
            else:
                self._exe = app

        configs = []
        for key, value in yml['include'].iteritems():
            log.debug('Configuring settings for including {}'.format(key))
            configs.append(update_config(value))
        return configs


    def _generate(self):
        """
        Generate the documentation.
        """

        # Setup the location
        log.info('Generating Documentation: {}'.format(self._config_file))

        # Parse the configuration file for the desired paths
        configs = self._configure()

        # Locate and run the MOOSE executable
        raw = utils.runExe(self._exe, '--yaml')
        ydata = utils.MooseYaml(raw)

        for config in configs:
            generator = MooseSubApplicationDocGenerator(ydata, config)
            generator.write()

        # Create the mkdocs.yml file
        def dumptree(node, level=0):
            for item in node:
                for key, value in item.iteritems():
                    if isinstance(value, list):
                        yield '{}- {}:'.format(' '*4*level, key)
                        for f in dumptree(value, level+1):
                            yield f
                    else:
                        yield '{}- {}: {}'.format(' '*4*(level), key, value)

        pages = MooseDocs.yaml_load('pages.yml')
        output = ['pages:']
        output += dumptree(pages, 1)
        shutil.copyfile('mkdocs.template.yml', 'mkdocs.yml')
        with open('mkdocs.yml', 'a') as fid:
            fid.write('\n'.join(output))
