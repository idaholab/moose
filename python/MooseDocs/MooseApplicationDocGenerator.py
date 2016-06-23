import os
import utils
from mkdocs.utils import yaml_load
import logging
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

    def __call__(self):
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

        # Read the general yml file (e.g. mkdocs.yml)
        with open(self._config_file, 'r') as fid:
            yml = yaml_load(fid.read())

        # Hide
        hide = yml.get('hide', list())
        install = yml.get('install', os.path.join(os.getcwd(), 'documentation'))

        def update_config(cname):
            """
            Helper for updating/creating local configuration dict.
            """

            # Open the local config file
            with open(cname) as fid:
                config = yaml_load(fid.read())

            # Defaults
            # Set the default source directory and sub-folder
            config['source'] = os.path.dirname(cname)
            config.setdefault('install', install)
            config['details'] = os.path.join(config['source'], config.get('details', os.path.join('docs', 'details')))
            config.setdefault('prefix', '')
            config.setdefault('repo', None)
            config.setdefault('doxygen', None)
            config.setdefault('hide', list())
            config.setdefault('links', dict())

            # Append the hide data
            config['hide'] = set(config['hide'] + hide)

            # Re-define the links path relative to working directory
            for key, value in config['links'].iteritems():
                out = []
                for path in value:
                    out.append(os.path.abspath(os.path.join(config['source'], path)))
                config['links'][key] = out

            return config

        # Extract executable
        if 'app' not in yml['extra']:
            self._exe = utils.find_moose_executable(self._root)
        else:
            app = yml['extra']['app']
            if os.path.isdir(app):
                self._exe = utils.find_moose_executable(app)
            else:
                self._exe = app

        configs = []
        for include in yml['extra']['include']:
            path = os.path.join(self._root, include, 'config.yml')
            configs.append(update_config(path))
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
