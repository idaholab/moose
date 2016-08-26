import os
import collections
import shutil
import logging
log = logging.getLogger(__name__)

from MooseSystemInformation import MooseSystemInformation
from MooseObjectInformation import MooseObjectInformation
from MooseApplicationSyntax import MooseApplicationSyntax
import database

class MooseSubApplicationDocGenerator(object):
    """
    Generate documentation for an application, for a given directory.

    Args:
        yaml_data[dict]: The complete YAML object obtained from the MOOSE application.
        filename[str]: The MkDocs yaml file to create.
        config[dict]: A dictionary with configuration options (see MooseApplicationDocGenerator).
    """

    def __init__(self, yaml_data, config):

        # Configuration
        self._config = config

        # Extract the input/source link directories to utilize and build databases
        links = self._config.get('links')
        hide = self._config.get('hide')

        # Create the database of input files and source code
        inputs = collections.OrderedDict()
        children = collections.OrderedDict()
        options = {'repo': self._config['repo']}
        log.info('Building input and inheritance databases...')
        for key, path in links.iteritems():
            inputs[key] = database.Database('.i', path, database.items.InputFileItem, **options)
            children[key] = database.Database('.h', path, database.items.ChildClassItem, **options)

        # Parse the syntax for the given source directory.
        src = self._config.get('source')
        log.info('Locating syntax for application: {}'.format(src))
        self._yaml_data = yaml_data
        self._syntax = MooseApplicationSyntax(yaml_data, src)

        self._systems = []
        log.info('Initializing MOOSE system information...')
        for system in self._syntax.systems():
            node = yaml_data.find(system)
            if not any([node['name'].startswith(h) for h in hide]):
                self._systems.append(MooseSystemInformation(node, self._syntax, **self._config))

        self._objects = []
        log.info('Initializing MooseObject information...')
        for key, value in self._syntax.objects().iteritems():
            src = self._syntax.filenames(key)
            nodes = yaml_data[key]
            for node in nodes:
                if not any([node['name'].startswith(h) for h in hide]):
                    self._objects.append(MooseObjectInformation(node, src, inputs=inputs, children=children, **self._config))

    def write(self):
        """
        Write the system and object markdown as well as the associated yaml files for mkdocs.
        """

        for system in self._systems:
            system.write()

        for obj in self._objects:
            obj.write()

        yml = self.generateYAML()
        if not yml:
            log.warning("No pages generated, the source location(s) must be empty.")
            return

        install_dir = self._config.get('install')
        if not os.path.exists(install_dir):
            log.info('Creating install directory: {}'.format(install_dir))
            os.mkdir(install_dir)

        filename = os.path.abspath(os.path.join(install_dir, 'pages.moose.yml'))
        log.info('Creating YAML file: {}'.format(filename))
        with open(filename, 'w') as fid:
            fid.write(yml)

    def generateYAML(self):
        """
        Generates the pages.moose.yml file.
        """

        install = self._config['install']

        rec_dd = lambda: collections.defaultdict(rec_dd)
        tree = rec_dd()
        for root, dirs, files in os.walk(install, topdown=True):

            if 'Overview.moose.md' in files:
                files.insert(0, files.pop(files.index('Overview.moose.md')))

            for filename in files:

                if not filename.endswith('.moose.md'):
                    continue

                name = filename[:-9]
                relative = os.path.relpath(root, install).split(os.path.sep)
                level = len(relative)
                cmd = "tree{}".format(("['{}']"*level).format(*relative))

                d = eval(cmd)
                if 'items' not in d:
                    d['items'] = []

                base = os.path.join(os.path.relpath(install, os.getcwd()), *relative)
                d['items'].append( (name, os.path.join(base, filename) ) )

        def dumptree(node, level=0):

            if 'items' in node:
                for item in node['items']:
                    yield '{}- {}: {}'.format(' '*4*(level), *item)
                node.pop('items')

            for key, value in node.iteritems():
                yield '{}- {}:'.format(' '*4*level, key)
                for f in dumptree(value, level+1):
                    yield f

        # Sort
        sorted_tree = collections.OrderedDict()
        for key in sorted(tree.keys()):
            sorted_tree[key] = tree[key]

        output = dumptree(sorted_tree)
        return '\n'.join(output)
