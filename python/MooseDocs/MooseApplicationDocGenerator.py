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
        self._develop = kwargs.pop('develop', False)

    def generate(self, purge=False):
        """
        Operator(). Calling this function causes the documentation to generated.

        Args:
            purge[bool]: When True the install directory is cleaned.
        """

        # Remove all '.moose.md/.moose.yml' files from the current directory
        if purge:
            log.info('Purging *.moose.md and *.moose.yml files from {}'.format(os.getcwd()))
            for root, dirs, files in os.walk(os.getcwd(), topdown=False):
                for name in files:
                    if name.endswith('.moose.md') or name.endswith('.moose.yml'):
                        full_file = os.path.join(root, name)
                        log.debug('Removing: {}'.format(full_file))
                        os.remove(full_file)

        """
        TODO: Tie this into the mkdocs livereload. To do this the mkdocs watching should be paused while
        the files are generated and then mkdocs livereload restarted. Otherwise, each file that changes
        spawns a rebuild.
        """
        self._generate()
        self._modified = os.path.getmtime(self._exe)

    def _configure(self):
        """
        Build the configuration options for included sub-directories.
        """

        # Read the moosedocs yml configuration file
        yml = MooseDocs.yaml_load(self._config_file)

        # Default settings
        defaults = yml.get('defaults', dict())
        defaults.setdefault('details', os.path.join(os.getcwd(), 'details'))
        defaults.setdefault('source', None)
        defaults.setdefault('install', os.path.join(os.getcwd(), 'content'))
        defaults.setdefault('repo', None)
        defaults.setdefault('doxygen', None)
        defaults.setdefault('hide', list())
        defaults.setdefault('links', dict())

        def update_config(config):
            """
            Helper for updating/creating local configuration dict.
            """

            # Apply defaults
            for key, value in defaults.iteritems():
                config.setdefault(key, value)

            # Append the hide data
            config['hide'] = set(config.get('hide', list()) + list(defaults['hide']))

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
        if 'include' in yml:
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
