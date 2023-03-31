#!/usr/bin/env python3
"""
Manipulate Apptainer/Harbor containers based on version/hashes of MOOSE repository
"""
import os
import sys
import argparse
import re
import socket
import subprocess
import json
import tempfile

import jinja2

from versioner import Versioner

MOOSE_DIR = os.environ.get('MOOSE_DIR',
                           os.path.abspath(os.path.join(os.path.dirname(
                               os.path.realpath(__file__)), '..')))

class ApptainerGenerator:
    """
    Generates apptainer containers needed for building
    MOOSE and MOOSE-based applications.
    """
    def __init__(self):
        self.meta = Versioner().meta()
        self.args = self.parse_args(list(self.meta.keys()))

        self.library_meta = self.meta[self.args.library]['apptainer'].copy()
        self.project = self.library_meta['name_base']
        self.name = self.library_meta['name']
        self.tag = self.library_meta['tag']

        if hasattr(self.args, 'modify') and self.args.modify is not None:
            self.def_path = os.path.abspath(self.args.modify)
        else:
            self.def_path = self.library_meta['def']

        if self.args.suffix is not None:
            self.name = self.add_name_suffix(self.library_meta, self.args.suffix)
        if self.args.tag is not None:
            self.tag = self.args.tag
        if self.args.tag_prefix is not None:
            self.tag = f'{self.args.tag_prefix}-{self.tag}'

        if hasattr(self.args, 'dir'):
            self.dir = os.path.abspath(self.args.dir)
            if not os.path.isdir(self.dir):
                self.error(f'Generation path {self.dir} does not exist')
        else:
            self.dir = None

    @staticmethod
    def parse_args(entities):
        """
        Parses command line arguments.
        """
        # pylint: disable=too-many-locals
        oras_url_default = 'mooseharbor.hpc.inl.gov'

        parser = argparse.ArgumentParser(description='Supplies a hash for a given library')

        parent = argparse.ArgumentParser(add_help=False)

        action_parser = parser.add_subparsers(dest='action', help='Action to perform')
        action_parser.required = True

        def add_default_args(parser, write=True, remote=False):
            parser.add_argument('library', choices=entities,
                    help='The library to act on')
            parser.add_argument('--suffix', type=str, help='Suffix to add to the name')
            parser.add_argument('--tag', type=str, help='Alternate tag')
            parser.add_argument('--tag-prefix', type=str, help='Prefix to add to the tag')
            if write:
                parser.add_argument('dir', help='The directory to perform actions in', default='.')
                parser.add_argument('--overwrite', action='store_true',
                                    help='Overwrite any containers')
            if remote:
                parser.add_argument('--oras-url', type=str, default=oras_url_default,
                                    help='The ORAS URL to use; ' +
                                        f'defaults to {oras_url_default}')
                parser.add_argument('--disable-cache', action='store_true',
                                    help='Disable the apptainer cache')

        exists_parser = action_parser.add_parser('exists', parents=[parent],
                                                 help='Checks if a container exists'
                                                 + ' on a remote')
        add_default_args(exists_parser, write=False, remote=True)

        def_parser = action_parser.add_parser('def', parents=[parent],
                                              help='Generates a definition')
        def add_def_args(parser):
            add_default_args(parser, remote=True)
            parser.add_argument('--local', action='store_true',
                                help='Use a local dependency container')
            parser.add_argument('--alt-dep-tag', type=str,
                                help='An alternate dependency tag to pull')
            parser.add_argument('--alt-dep-tag-prefix', type=str,
                                help='A prefix to add to the alternate dependency tag')
            parser.add_argument('--dep-suffix', type=str)
            parser.add_argument('--modify', type=str,
                                help='Modify the container instead; path to def template')
        add_def_args(def_parser)

        pull_parser = action_parser.add_parser('pull', parents=[parent],
                                               help='Pull a container')
        add_default_args(pull_parser, remote=True)
        pull_parser.add_argument('--pull-args', type=str,
                                 help="Arguments to pass to apptainer pull")

        path_parser = action_parser.add_parser('path', parents=[parent],
                                              help='Get the local path to a container')
        add_default_args(path_parser, write=False)

        build_parser = action_parser.add_parser('build', parents=[parent],
                                                help='Build a container')
        add_def_args(build_parser)
        build_parser.add_argument('--build-args', type=str,
                                  help="Arguments to pass to apptainer build")
        build_parser.add_argument('--sign', type=int,
                                  help='Sign the built container with the given key')

        squash_parser = action_parser.add_parser('squash', parents=[parent],
                                                 help='Squash a container (app only)')
        add_default_args(squash_parser)

        push_parser = action_parser.add_parser('push', parents=[parent],
                                                help='Push a container')
        add_default_args(push_parser, remote=True)
        push_parser.add_argument('--to-tag', type=str,
                                 help='An alternate tag to push to')
        push_parser.add_argument('--to-tag-prefix', type=str,
                                 help='A prefix to add to the pushed tag')
        push_parser.add_argument('--squashed', action='store_true',
                                 help='Push the squashed container (app only)')

        uri_parser = action_parser.add_parser('uri', parents=[parent],
                                              help='Get the URI to a container')
        add_default_args(uri_parser, write=False, remote=True)
        uri_parser.add_argument('--check', action='store_true',
                                help='Check whether or not the container exists')

        return parser.parse_args()

    @staticmethod
    def add_color(content: str, prefix_color: str):
        """
        Adds color (for on-screen output) to the given content
        """
        prefix_color_vals = {'red': 31, 'green': 32, 'yellow': 33}
        if prefix_color not in prefix_color_vals:
            ApptainerGenerator.error('Unknown prefix color {}'.format(prefix_color))
        return '\033[{}m{}\033[0m'.format(prefix_color_vals[prefix_color], content)

    @staticmethod
    def print(content, prefix_color='green', file=sys.stdout):
        """
        Helper for on-screen output with a prefix
        """
        prefix = '[' + ApptainerGenerator.add_color('\033[1mgenerator\033[0m', prefix_color) + ']'
        print(prefix, content, file=file, flush=True)

    @staticmethod
    def warn(content):
        """
        Helper for on-screen output with a yellow prefix for a warning
        """
        ApptainerGenerator.print(content, prefix_color='yellow')

    @staticmethod
    def error(content):
        """
        Helper for red on-screen output in the event
        of an error followed by a non-zero exit
        """
        ApptainerGenerator.print(content, prefix_color='red', file=sys.stderr)
        sys.exit(1)

    def run(self, command):
        """
        Prints a command to screen and then runs it
        """
        self.print(self.add_color(' '.join(command), 'green'))
        subprocess.run(command, check=True)

    def container_path(self, name: str, tag: str, ext='sif'):
        """
        Gets the local path to a container or definition file
        """
        return os.path.join(self.dir, f'{name}_{tag}.{ext}')

    def oras_uri(self, project: str, name: str, tag: str):
        """
        Gets the ORAS URI for the given image
        """
        return f'oras://{self.args.oras_url}/{project}/{name}:{tag}'

    def apptainer_pull(self, project: str, name: str, tag: str, args=None):
        """
        Pulls the given image via apptainer
        """
        oras_uri = self.oras_uri(project, name, tag)
        file = self.container_path(name, tag)
        self.print(f'Pulling {oras_uri}')

        command = ['apptainer', 'pull']
        if args is not None:
            command += args
        if (hasattr(self.args, 'disable_cache') and
            self.args.disable_cache and
            '--disable-cache' not in command):
            command += ['--disable-cache']
        command += [file, oras_uri]
        self.run(command)
        return file

    def apptainer_sign(self, name: str, tag: str, key_id: int):
        """
        Signs the given image vit apptainer
        """
        container_path = self.container_path(name, tag)
        self.print(f'Signing {container_path}')
        command = ['apptainer', 'sign', '-k', str(key_id), container_path]
        self.run(command)

    def apptainer_build(self, def_file, name, tag, args=None):
        """
        Builds the given image via apptainer
        """
        file = self.container_path(name, tag)
        self.print(f'Building {def_file} in {file}')
        command = ['apptainer', 'build', '--fakeroot']
        if args is not None:
            command += args
        if (hasattr(self.args, 'disable_cache') and
            self.args.disable_cache and
            '--disable-cache' not in command):
            command += ['--disable-cache']
        command += [file, def_file]
        self.run(command)

    def oras_call(self, command, loud=True, check=True, cwd=os.getcwd()):
        config_file = os.path.join(os.environ['HOME'], '.apptainer/docker-config.json')
        command = ['oras', '--registry-config', config_file] + command

        if loud:
            stdout = sys.stdout
            stderr = sys.stderr
            self.print(self.add_color(' '.join(command), 'green'))
        else:
            stdout = subprocess.PIPE
            stderr = subprocess.PIPE

        return subprocess.run(command, stdout=stdout, stderr=stderr, check=check, cwd=cwd)

    def oras_exists(self, uri: str):
        """
        Checks whether or not the given image exists via ORAS
        """
        command = ['manifest', 'fetch', uri.replace('oras://', '')]
        process = self.oras_call(command, loud=False, check=False)
        if process.returncode == 0:
            return True

        if 'not found' in process.stderr.decode("utf-8"):
            return False

        print(process.stderr.decode("utf-8"), file=sys.stderr)
        raise Exception('Failed to check ORAS image existance')

    def oras_push(self, project: str, name: str, tag: str, files: list, extra_annotations={}):
        """
        Pushes the given files via oras
        """
        oras_uri = self.oras_uri(project, name, tag).replace('oras://', '')
        oras_command = ['push', oras_uri]

        annotations = {}
        for file in files:
            if not file.startswith(self.dir):
                self.error(f'File {file} does not start with generation dir {self.dir}')
            filetype_suffix = ''
            if file.endswith('.sif'):
                annotations = self.build_oras_annotations(file, extra_annotations=extra_annotations)
                filetype_suffix = ':application/vnd.sylabs.sif.layer.v1.sif'
            oras_command.append(os.path.basename(file) + filetype_suffix)

        with tempfile.NamedTemporaryFile(mode='w') as annotation_tf:
            if annotations is not None:
                with annotation_tf.file as tf:
                    tf.write(json.dumps(annotations))
                oras_command.extend(['--annotation-file', annotation_tf.name])

            self.oras_call(oras_command, cwd=self.dir)

    @staticmethod
    def add_name_suffix(meta, suffix):
        """
        Adds a suffix to the name for the given library meta
        """
        current_suffix = meta['name_suffix']
        new_suffix = suffix + '-' + meta['name_suffix']
        return meta['name'].replace(current_suffix, new_suffix)

    def _find_dependency_meta(self, library):
        """
        Find the dependency meta for the given library (if any)
        """
        all_libraries = list(self.meta.keys())
        for i in range(1, len(all_libraries)):
            if all_libraries[i] == library:
                return self.meta[all_libraries[i - 1]]['apptainer']
        return None

    def _dependency_from(self, meta):
        """
        Finds the BootStrap and From options based on the given dependency
        """
        project = meta['name_base']
        name = meta['name']
        tag = meta['tag']
        if self.args.dep_suffix is not None:
            name = self.add_name_suffix(meta, self.args.dep_suffix)

        # localimage (with --local option)
        if self.args.local:
            container_path = self.container_path(name, tag)
            if not os.path.exists(container_path):
                self.error(f'Dependency {name} does not exist in {container_path}')
            self.print(f'Using local dependency for {name} in {container_path}')
            return 'localimage', container_path

        # First, try the production tag
        uri = self.oras_uri(project, name, tag)
        prod_exists = self.oras_exists(uri)
        # Failed to find a production tag
        if not self.oras_exists(uri):
            not_found = f'Remote container dependency {uri} not found'
            # Without an alternate, we're screwed
            if self.args.alt_dep_tag is None and self.args.alt_dep_tag_prefix is None:
                self.error(not_found)

            self.warn(not_found + '; trying alternate')

            # We have an alternate tag (example: trying to pull a PR); try it
            if self.args.alt_dep_tag is not None:
                tag = self.args.alt_dep_tag
            if self.args.alt_dep_tag_prefix is not None:
                tag = f'{self.args.alt_dep_tag_prefix}-{tag}'
            uri = self.oras_uri(project, name, tag)
            alt_exists = self.oras_exists(uri)

            # No luck, we're still screwed
            if not alt_exists:
                self.error(f'Alternate container dependency {uri} not found')
        if (not prod_exists
           and self.args.alt_dep_tag is None
           and self.args.alt_dep_tag_prefix is None):
            self.error(f'Remote container dependency {uri} not found')

        self.print(f'Using remote dependency for {name} from {uri}')
        return 'oras', uri.replace('oras://', '')

    @staticmethod
    def submodule_sha(cwd, name):
        """
        Get the SHA of the given submodule
        """
        cmd = ['git', 'ls-tree', '--object-only', 'HEAD', name]
        return subprocess.check_output(cmd, cwd=cwd, encoding='utf-8').rstrip()

    def _add_definition_labels(self, definition):
        """
        Adds common labels to the given definition content
        """
        # Get the moose sha
        moose_sha_command = ['git', 'rev-parse', 'HEAD']
        moose_sha = subprocess.check_output(moose_sha_command, encoding='utf-8', cwd=MOOSE_DIR).rstrip()

        definition += '\n\n%labels\n'
        def add_label(key, value):
            name = 'moose.' + self.project
            if hasattr(self.args, 'modify') and self.args.modify is not None:
                name += '.modified'
            return f'    {name}.{key} {value}\n'

        definition += add_label('build.host', socket.gethostname())
        definition += add_label('build.user', os.getenv('USER'))
        definition += add_label('moose.gitsha', moose_sha)
        definition += add_label('version', self.library_meta['tag'])

        # If we have CIVET info, add the url
        if 'CIVET_SERVER' in os.environ and 'CIVET_JOB_ID' in os.environ:
            civet_server = os.environ.get('CIVET_SERVER')
            civet_job_id = os.environ.get('CIVET_JOB_ID')
            definition += add_label('build.civetjob', f'{civet_server}/job/{civet_job_id}')

        # For libmesh and petsc, also add the commit
        if self.args.library in ['petsc', 'libmesh']:
            definition += add_label(f'{self.args.library}.gitsha',
                                    self.submodule_sha(MOOSE_DIR, self.args.library))

        return definition

    @staticmethod
    def create_filename(app_root, section_key, actions):
        """
        Build and return a list of (sections, file_path) tuples
        Returns:
            [('SECTION_METHOD_ACTION','approot/apptainer/section_method_action.sh')]
        """
        file_list = []
        # Support sections that have neither method or action (like environment.sh)
        if os.path.exists(os.path.join(app_root,
                                       'apptainer',
                                       f'{section_key}.sh')):
            file_list = [(f'SECTION_{section_key.upper()}',
                         os.path.join(app_root, 'apptainer', f'{section_key}.sh'))]

        for method in ['pre', 'post']:
            # Support sections that have no action (like post_pre.sh)
            if os.path.exists(os.path.join(app_root,
                                           'apptainer',
                                           f'{section_key}_{method}.sh')):
                file_list.append((f'SECTION_{section_key.upper()}_{method.upper()}',
                                  os.path.join(app_root,
                                               'apptainer',
                                               f'{section_key}_{method}.sh')))

            for action in actions:
                file_path = os.path.join(app_root,
                                        'apptainer',
                                        f'{section_key}_{method}_{action}.sh')
                # Support sections that have method and an action (post_pre_configure.sh)
                if os.path.exists(file_path):
                    file_list.append((f'SECTION_{section_key.upper()}_'
                                      f'{method.upper()}_{action.upper()}',
                                      file_path))
        return file_list

    def add_application_additions(self, app_root, jinja_data):
        """
        Add application specific sections if found to jinja data
        Example, if found:
            app_root/apptainer/post_pre_configure.sh
            app_root/apptainer/post_pre.sh
            app_root/apptainer/environment.sh
            etc
        then the contents therein will be exchanged where appropriate.

        File naming syntax:
            <section>_<method>_<action>.sh
        """
        # Add your sections here (be sure to modify apptainer/app.def to match)
        section_meta = {'environment': [],
                        'post': ['configure', 'make', 'makeinstall'],
                        'test': ['test']}
        for section_key, actions in section_meta.items():
            file_list = self.create_filename(app_root, section_key, actions)
            for (a_section, file_path) in file_list:
                with open(file_path, 'r') as section_file:
                    multi_lines = []
                    for i, a_line in enumerate(section_file.readlines()):
                        # next line items need indentation
                        if i != 0:
                            multi_lines.append(f'    {a_line}')
                        else:
                            multi_lines.append(a_line)
                    jinja_data[a_section] = ''.join(multi_lines)

    def add_definition_vars(self, jinja_data):
        """
        Adds conditional apptainer definition vars to jinja data
        """
        # Set application-related variables
        if self.args.library == 'app':
            app_name, app_root, _ = Versioner.get_app()
            jinja_data['APPLICATION_DIR'] = app_root
            jinja_data['APPLICATION_NAME'] = os.path.basename(app_root)
            jinja_data['BINARY_NAME'] = app_name
            self.add_application_additions(app_root, jinja_data)

        # Set MOOSE_[TOOLS, TEST_TOOLS]_VERSION
        if self.args.library == 'moose':
            for package in ['tools', 'test-tools']:
                meta_yaml = os.path.join(MOOSE_DIR, f'conda/{package}/meta.yaml')
                with open(meta_yaml, 'r') as meta_contents:
                    _, version, _, _ = Versioner.conda_meta_jinja(meta_contents.read())
                    variable_name = 'MOOSE_'
                    variable_name += package.upper().replace('-', '_')
                    variable_name += '_VERSION'
                    jinja_data[variable_name] = version
        elif self.args.library == 'libmesh':
            package = 'libmesh-vtk'
            meta_yaml = os.path.join(MOOSE_DIR, f'conda/{package}/meta.yaml')
            with open(meta_yaml, 'r') as meta_contents:
                _, _, _, meta = Versioner.conda_meta_jinja(meta_contents.read())
            for var in ['url', 'sha256', 'vtk_friendly_version']:
                jinja_var = f'vtk_{var}'
                jinja_data[jinja_var] = meta['source'][var]

    @staticmethod
    def get_sif_labels(sif_path):
        inspect_cmd = ['apptainer', 'inspect', '-j', sif_path]
        inspection = json.loads(subprocess.check_output(inspect_cmd).decode(sys.stdout.encoding))
        return inspection['data']['attributes'].get('labels', {})

    @staticmethod
    def build_oras_annotations(container_path, extra_annotations={}):
        annotations = {'$manifest': {}}
        annotations['$manifest'].update(ApptainerGenerator.get_sif_labels(container_path))
        annotations['$manifest'].update(extra_annotations)
        return annotations

    def _action_exists(self):
        """
        Performs the "exists" action
        """
        uri = self.oras_uri(self.project, self.name, self.tag)
        if self.oras_exists(uri):
            self.print(f'Container {uri} exists')
        else:
            self.error(f'Container {uri} does not exist')

    def _action_pull(self):
        """
        Performs the "pull" action
        """
        container_path = self.container_path(self.name, self.tag)
        if os.path.exists(container_path):
            if self.args.overwrite:
                self.warn(f'Deleting container {container_path}')
                os.remove(container_path)
            else:
                self.error(f'Container already exists in {container_path}')

        uri = self.oras_uri(self.project, self.name, self.tag)
        if not self.oras_exists(uri):
            self.error(f'Container {uri} does not exist')

        args = [] if self.args.pull_args is None else self.args.pull_args.split(' ')
        self.apptainer_pull(self.project, self.name, self.tag, args)

    def _action_def(self):
        """
        Performs the "def" action
        """
        definition = open(self.def_path, 'r').read()

        self.print(f'Building definition for {self.name}:{self.tag}')

        # Prepare the jinja data for filling the definitions
        jinja_data = dict(os.environ)
        jinja_data['MOOSE_DIR'] = MOOSE_DIR

        # Find the dependent library (if any)
        if self.args.modify is None:
            dep_meta = self._find_dependency_meta(self.args.library)
        else:
            self.print(f'Modifying container with definition {self.def_path}')
            dep_meta = self.meta[self.args.library]['apptainer']

        # Whether or not the definition file has a dependency
        needs_from = '{{ APPTAINER_BOOTSTRAP }}' in definition
        # No dependent library needed
        if dep_meta is None:
            if needs_from:
                self.error(f'Library {self.name} needs a dependency, but none found')
        # Dependency library is needed, figure out how to get it
        else:
            if not needs_from:
                self.error(f'Definition {self.def_path} missing a templated BootStrap')

            apptainer_bootstrap, apptainer_from = self._dependency_from(dep_meta)
            jinja_data['APPTAINER_BOOTSTRAP'] = apptainer_bootstrap
            jinja_data['APPTAINER_FROM'] = apptainer_from

        # Add extra conditional vars
        self.add_definition_vars(jinja_data)

        # Use jinja to fill the definition file
        jinja_env = jinja2.Environment()
        definition_template = jinja_env.from_string(definition)
        jinja_env.trim_blocks = True
        jinja_env.lstrip_blocks = True
        new_definition = definition_template.render(**jinja_data)

        # Add in a few labels
        new_definition = self._add_definition_labels(new_definition)

        # Definition file checks
        container_definition_path = self.container_path(self.name, self.tag, ext='def')
        self.print(f'Writing definition to {container_definition_path}')
        if os.path.exists(container_definition_path):
            if self.args.overwrite:
                self.warn(f'Deleting definition {container_definition_path}')
                os.remove(container_definition_path)
            else:
                self.error(f'Definition {container_definition_path} already exists')

        # Write the definition file
        open(container_definition_path, 'w').write(new_definition)

    def _action_build(self):
        """
        Performs the "build" action
        """
        # Build the definnition
        self._action_def()

        container_path = self.container_path(self.name, self.tag)

        # The .sif better not exist
        if os.path.exists(container_path):
            if self.args.overwrite:
                self.warn(f'Deleting container {container_path}')
                os.remove(container_path)
            else:
                self.error(f'Container already exists in {container_path}')

        # Do the build!
        args = []
        if self.args.build_args is not None:
            args = self.args.build_args.split(' ')
        container_definition_path = self.container_path(self.name, self.tag, ext='def')
        self.apptainer_build(container_definition_path, self.name, self.tag, args=args)

        # Sign if requested
        if self.args.sign is not None:
            self.apptainer_sign(self.name, self.tag, self.args.sign)

    def _action_squash(self):
        """
        Performs the "squash" action
        """
        if self.args.library != 'app':
            self.error('Can only squash applications')

        container_path = self.container_path(self.name, self.tag)
        if not os.path.exists(container_path):
            self.error(f'Container {container_path} does not exist')
        self.print(f'Squashing container {container_path}')

        squashfs_path = self.container_path(self.name, self.tag, ext='squashfs')
        if os.path.exists(squashfs_path):
            if self.args.overwrite:
                self.warn(f'Overwriting {squashfs_path}')
                os.remove(squashfs_path)
            else:
                self.error(f'Squashed filesystem {squashfs_path} already exists')

        bind_mount = os.path.dirname(squashfs_path) + ':' + os.path.dirname(squashfs_path)
        command = ['apptainer', 'exec', '-B', bind_mount, container_path, '/opt/moose-apptainer/squash.sh', squashfs_path]
        self.run(command)

        self.print(f'Container was squashed to {squashfs_path}')

    def _action_push(self):
        """
        Performs the "push" action
        """
        from_tag = self.tag
        to_tag = from_tag if self.args.to_tag is None else self.args.to_tag
        if self.args.to_tag_prefix is not None:
            to_tag = f'{self.args.to_tag_prefix}-{to_tag}'

        container_path = self.container_path(self.name, from_tag)
        if not os.path.exists(container_path):
            self.error(f'Container {container_path} does not exist')

        if self.args.squashed:
            squashfs_path = self.container_path(self.name, from_tag, ext='squashfs')
            if not os.path.exists(squashfs_path):
                self.error(f'To-be squashed container {squashfs_path} does not exist')

            # Load the def file from the container
            dep_def_cmd = ['apptainer', 'inspect', '-d', container_path]
            dep_def = subprocess.check_output(dep_def_cmd).decode(sys.stdout.encoding)

            # Find the container that this was based on
            dep_from_re = re.search(r'From: +(([a-zA-Z0-9_.]+)\/([a-zA-Z0-9_.-]+)\/([a-zA-Z0-9_.-]+)\:([a-zA-Z0-9_.-]+))', dep_def)
            if not dep_from_re:
                self.error(f'Failed to parse from-container from def for {container_path}')
            dep_uri = 'oras://' + dep_from_re.group(1)
            dep_host = dep_from_re.group(2)
            dep_project = dep_from_re.group(3)
            dep_name = dep_from_re.group(4)
            dep_tag = dep_from_re.group(5)

            # Get the container that this was based on
            self.print(f'Fetching dependent container {dep_name}:{dep_tag}')
            from_container_path = self.container_path(dep_name, dep_tag)
            if os.path.exists(from_container_path):
                if self.args.overwrite:
                    self.warn(f'Overwriting {from_container_path}')
                    os.remove(from_container_path)
                else:
                    self.error(f'From container {from_container_path} already exists')
            self.apptainer_pull(dep_project, dep_name, dep_tag)

            # Add "-squashed" to the end of the name
            name_arch = self.name.split('-')[-1]
            to_name = self.name.replace(name_arch, 'squashed-' + name_arch)

            # Make sure it doesn't already exist
            oras_uri = self.oras_uri(self.project, to_name, to_tag)
            if self.oras_exists(oras_uri):
                if self.args.overwrite:
                    self.warn(f'Overwriting {oras_uri}')
                else:
                    self.error(f'Tag {oras_uri} already exists')

            # Fill the extra annotations (the labels from the app container)
            extra_annotations = {}
            app_labels = self.get_sif_labels(container_path)
            for key, value in app_labels.items():
                if key.startswith(self.library_meta['name_base']):
                    extra_annotations[key] = value

            self.print(f'Pushing squashed container {to_name}:{to_tag}')
            self.oras_push(self.project, to_name, to_tag, [squashfs_path, from_container_path],
                           extra_annotations=extra_annotations)
        else:
            uri = self.oras_uri(self.project, self.name, to_tag)
            if self.oras_exists(uri):
                if self.args.overwrite:
                    self.warn(f'Overwriting {uri}')
                else:
                    self.error(f'Tag {uri} already exists')

            self.oras_push(self.project, self.name, to_tag, [container_path])

    def _action_path(self):
        """
        Performs the "path" action
        """
        print(self.container_path(self.name, self.tag))

    def _action_uri(self):
        """
        Performs the "uri" action
        """
        uri = self.oras_uri(self.project, self.name, self.tag)
        if self.args.check and not self.oras_exists(uri):
            self.error(f'Container {uri} does not exist')
        print(uri)

    def main(self):
        """
        Runs the specificed CLI action
        """
        getattr(self, f'_action_{self.args.action}')()

if __name__ == '__main__':
    ApptainerGenerator().main()
