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
        self._develop = kwargs.pop('develop', False)

    def generate(self):
        """
        Generate the documentation.
        """

        # Setup the location
        log.info('Generating Documentation: {}'.format(self._config_file))

        # Cache executable
        exe_cache = None

        # Parse the configuration file and build sub application generators
        configs = self._configure()
        for config in configs:

            # Extract executable
            if 'executable' not in config:
                exe = utils.find_moose_executable(self._root)
            else:
                exe = config['executable']
                if os.path.isdir(exe):
                    exe = utils.find_moose_executable(exe)

            # Error if the executable is not found
            if not os.path.exists(exe):
                log.error('Unable to locate a working executable: {}'.format(self._exe))
                return

            # Locate and run the MOOSE executable (if it hasn't been run already)
            if exe != exe_cache:
                exe_cache = exe
                log.debug("Executing {} to extract syntax.".format(exe))
                raw = utils.runExe(exe, '--yaml')
                ydata = utils.MooseYaml(raw)

            # Generate the documentation
            generator = MooseSubApplicationDocGenerator(ydata, config)
            generator.write()

    def _configure(self):
        """
        Build the configuration options for included sub-directories.
        """

        # Read the moosedocs yml configuration file
        yml = MooseDocs.yaml_load(self._config_file)

        if 'extra' not in yml:
            log.error("Unable to locate configuration in {}".format(self._config_file))
            return []

        # Default settings
        defaults = yml['extra'].pop('defaults', dict())
        defaults.setdefault('executable', None)
        defaults.setdefault('details', os.path.join(os.getcwd(), 'details'))
        defaults.setdefault('source', None)
        defaults.setdefault('include', None)
        defaults.setdefault('install', os.path.join(os.getcwd(), 'content'))
        defaults.setdefault('repo', None)
        defaults.setdefault('doxygen', None)
        defaults.setdefault('hide', list())
        defaults.setdefault('links', dict())
        print defaults

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

        configs = []
        for key, value in yml['extra'].iteritems():
            log.debug('Configuring settings for including {}'.format(key))
            configs.append(update_config(value))
        return configs
