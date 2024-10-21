#!/usr/bin/env python3
"""
Manipulate Apptainer/Harbor containers based on version/hashes of MOOSE repository
"""
import os
import sys
import argparse
import socket
import subprocess
import shutil
import platform
import getpass
import re
from datetime import datetime, timezone

import jinja2
from jinja2 import meta

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
        self.meta = Versioner().version_meta()

        # Get the packages that have an 'apptainer' key in versioner.yaml
        apptainer_packages = []
        for library, values in self.meta.items():
            if 'apptainer' in values:
                apptainer_packages.append(library)

        self.args = self.parse_args(apptainer_packages)
        self.args = self.verify_args(self.args)

        library_meta = self.meta[self.args.library]['apptainer'].copy()
        self.project = library_meta['name_base']
        self.name = library_meta['name']
        self.tag = library_meta['tag']
        self.version = library_meta['tag']

        if hasattr(self.args, 'modify') and self.args.modify is not None:
            self.def_path = os.path.abspath(self.args.modify)
        else:
            self.def_path = library_meta['def']

        if getattr(self.args, 'suffix', None):
            self.name = self.add_name_suffix(library_meta, self.args.suffix)
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
    def verify_args(args):
        """
        Verify arguments are sane.
        TODO: for now, this only checks for the presence of required executables
        """
        error_list = []
        # [(binary name, check for existence)] list of tuples
        requirements = [('oras', args.action=='def' and not args.local),
                        ('apptainer', args.action!='def')]
        for (requirement, check) in requirements:
            if check and shutil.which(requirement) is None:
                error_list.append(f'{requirement} executable not found')
        if error_list:
            print('/n'.join(error_list))
            sys.exit(1)
        return args

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

        def add_default_args(parser, write=True, remote=False, remote_fetch=False, repo=True):
            parser.add_argument('library', choices=entities,
                    help='The library to act on')
            if repo:
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
            if remote_fetch:
                parser.add_argument('--project-suffix', type=str,
                                    help='Suffix to add to the project')

        exists_parser = action_parser.add_parser('exists', parents=[parent],
                                                 help='Checks if a container exists'
                                                 + ' on a remote')
        add_default_args(exists_parser, write=False, remote=True, remote_fetch=True)

        def_parser = action_parser.add_parser('def', parents=[parent],
                                              help='Generates a definition')
        def add_def_args(parser):
            add_default_args(parser, remote=True, remote_fetch=True)
            parser.add_argument('--local', action='store_true',
                                help='Use a local dependency container')
            parser.add_argument('--dep', type=str,
                                help='Use this dependency instead')
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
        add_default_args(pull_parser, remote=True, remote_fetch=True)
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
        build_parser.add_argument('--skip-tests', action='store_true',
                                  help='Set to skip running the tests after the build')
        build_parser.add_argument('--no-cleanup', action='store_true',
                                  help='Pass to apptainer to not cleanup the build (for debugging)')

        push_parser = action_parser.add_parser('push', parents=[parent],
                                                help='Push a container')
        add_default_args(push_parser, remote=True)
        push_parser.add_argument('--to-tag', type=str,
                                 help='An alternate tag to push to')
        push_parser.add_argument('--to-tag-prefix', type=str,
                                 help='A prefix to add to the pushed tag')
        push_parser.add_argument('--to-project-suffix', type=str,
                                 help='A suffix to add to the project to push to')

        uri_parser = action_parser.add_parser('uri', parents=[parent],
                                              help='Get the URI to a container')
        add_default_args(uri_parser, write=False, remote=True, remote_fetch=True)
        uri_parser.add_argument('--check', action='store_true',
                                help='Check whether or not the container exists')

        tag_parser = action_parser.add_parser('tag', parents=[parent],
                                              help='Get the tag for a container')
        add_default_args(tag_parser, write=False, repo=False)

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

    @staticmethod
    def git_repo_sha(dir):
        """ gets sha of the given repo """
        command = ['git', 'rev-parse', 'HEAD']
        return subprocess.check_output(command, cwd=dir, encoding='utf-8').strip()

    @staticmethod
    def git_submodule_sha(dir, name):
        """ gets the sha of the given submodule """
        command = ['git', 'submodule', 'status', name]
        result = subprocess.check_output(command, cwd=dir, encoding='utf-8').strip()
        sha_re = re.search(r'^[U+-]?([a-f0-9]{40}) ', result)
        if sha_re:
            return sha_re.group(1)
        raise Exception(f'Failed to parse submodule sha for {name} from "{result}"')

    @staticmethod
    def git_submodule_remote(dir, name):
        """ gets the remote of the given submodule """
        command = ['git', 'config', '--file=.gitmodules', f'submodule.{name}.url']
        remote = subprocess.check_output(command, cwd=dir, encoding='utf-8').strip()

        # Need to replace remotes that are relative paths
        # Here, we'll replace even SSH relative paths with HTTPS relative paths
        # because we shouldn't have private dependencies and SSH keys probably
        # aren't available in the build
        if remote.startswith('../..'):
            base_command = ['git', 'remote', 'get-url', 'origin']
            base_remote = subprocess.check_output(base_command, cwd=dir, encoding='utf-8').strip()
            host_re = re.search(r'^git@([a-zA-Z0-9_.-]+\.[a-zA-Z]+):', base_remote)
            if not host_re:
                host_re = re.search(r'^https:\/\/([a-zA-Z0-9_.-]+\.[a-zA-Z]+)\/', base_remote)
            if not host_re:
                raise Exception(f'Failed to replace ../../ in git submodule remote for {name}')
            return remote.replace('../../', f'https://{host_re.group(1)}/')

        return remote

    def run(self, command):
        """
        Prints a command to screen and then runs it
        """
        self.print(self.add_color(' '.join(command), 'green'))
        subprocess.run(command, check=True)

    def container_path(self, name: str, tag: str, image=True):
        """
        Gets the local path to a container or definition file
        """
        ext = 'sif' if image else 'def'
        return os.path.join(self.dir, f'{name}_{tag}.{ext}')

    def oras_uri(self, project: str, name: str, tag: str, project_suffix=None):
        """
        Gets the ORAS URI for the given image
        """
        if project_suffix is not None:
            project = f'{project}-{project_suffix}'
        return f'oras://{self.args.oras_url}/{project}/{name}:{tag}'

    def apptainer_pull(self, project: str, name: str, tag: str, project_suffix=None, args=None):
        """
        Pulls the given image via apptainer
        """
        oras_uri = self.oras_uri(project, name, tag, project_suffix=project_suffix)
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

    def apptainer_push(self, project: str, name: str, from_tag: str, to_tag=None, project_suffix=None):
        """
        Pushes the given image via apptainer
        """
        if to_tag is None:
            to_tag = from_tag
        oras_uri = self.oras_uri(project, name, to_tag, project_suffix=project_suffix)
        file = self.container_path(name, from_tag)
        self.print(f'Pushing {file}')
        command = ['apptainer', 'push', file, oras_uri]
        self.run(command)

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

    @staticmethod
    def oras_exists(uri: str):
        """
        Checks whether or not the given image exists via ORAS
        """
        config_file = os.path.join(os.environ['HOME'], '.apptainer/docker-config.json')
        uri = uri.replace('oras://', '')
        command = ['oras', 'manifest', 'fetch', '--registry-config', config_file, uri]
        process = subprocess.run(command, stdout=subprocess.PIPE,
                                 stderr=subprocess.PIPE, check=False)
        if process.returncode == 0:
            return True

        if 'not found' in process.stderr.decode("utf-8"):
            return False

        print(process.stderr.decode("utf-8"), file=sys.stderr)
        raise Exception('Failed to check ORAS image existance')

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
        apptainer_meta = self.meta[library]['apptainer']
        if 'from' in apptainer_meta:
            return self.meta[apptainer_meta['from']]['apptainer']
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
        uri = self.oras_uri(project, name, tag, project_suffix=self.args.project_suffix)
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
            uri = self.oras_uri(project, name, tag, project_suffix=self.args.project_suffix)
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

    def _definition_header(self, jinja_data):
        """
        Adds a useful header to generated definitions
        """
        definition = open(self.def_path, 'r').read()
        jinja_env = jinja2.Environment()
        jinja_vars = meta.find_undeclared_variables(jinja_env.parse(definition))

        arguments = ' '.join(sys.argv[1:])

        header = '#\n'
        header += '# Generated via MOOSE ApptainerGenerator (scripts/apptainer_generator.py)\n'
        header += '#\n'
        header += f'#   Arguments: {arguments}\n'
        header += f'#   Template:  {os.path.relpath(self.def_path, MOOSE_DIR)}\n'
        header += '#\n'
        header += '# Consumed jinja variables\n'
        header += '#\n'
        for var in sorted(jinja_vars):
            contents = f'"{jinja_data.get(var, "")}"'
            if '\n' in contents:
                contents = 'Multi-line string'
            header += f'#   {var}={contents}\n'
        header += '#\n'
        header += '\n'

        return header

    def _add_definition_labels(self, definition):
        """
        Adds common labels to the given definition content
        """
        contents = '%labels\n'
        name = self.name
        if hasattr(self.args, 'modify') and self.args.modify is not None:
            name += '.modified'
        contents += f'{name}.host.hostname {socket.gethostname()}\n'
        contents += f'{name}.host.user {getpass.getuser()}\n'
        contents += f'{name}.moose.sha {self.git_repo_sha(MOOSE_DIR)}\n'
        contents += f'{name}.version {self.tag}\n'
        # If we have CIVET info, add the url
        if 'CIVET_SERVER' in os.environ and 'CIVET_JOB_ID' in os.environ:
            civet_server = os.environ.get('CIVET_SERVER')
            civet_job_id = os.environ.get('CIVET_JOB_ID')

            # We use the backend server on most build clients, so replace that
            # URL with something more useful if we can
            civet_server = civet_server.replace('https://civet-be.', 'https://civet.')

            contents += f'{name}.civet.job {civet_server}/job/{civet_job_id}\n'
        return definition + '\n\n' + self.add_def_whitespace(contents)

    def _add_definition_environment(self, definition):
        """
        Adds to the definition environment
        """
        if self.version == self.tag:
            name_summary = f'{self.name}:{self.tag}'
        else:
            name_summary = f'{self.name}:{self.tag}({self.version})'

        content = [f'#',
                   f'# Begin environment for {name_summary}',
                   f'#',
                   f'export MOOSE_APPTAINER_GENERATOR_LIBRARY="{self.args.library}"',
                   f'export MOOSE_APPTAINER_GENERATOR_NAME="{self.name}"',
                   f'export MOOSE_APPTAINER_GENERATOR_NAME_SUMMARY="{name_summary}"',
                   f'export MOOSE_APPTAINER_GENERATOR_TAG="{self.tag}"',
                   f'export MOOSE_APPTAINER_GENERATOR_VERSION="{self.version}"']
        content = '\n    ' + '\n    '.join(content) + '\n\n'

        env_header = '\n%environment\n'
        if env_header in definition:
            definition = definition.replace(env_header, env_header + content)
        else:
            definition += env_header + content

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

    @staticmethod
    def add_def_whitespace(content):
        """
        Adds whitespace to content for adding to a defintion file
        """
        if not content:
            return content
        return content.replace('\n', '\n    ')

    def add_definition_includes(self, jinja_data):
        """
        Includes files from the repository into the definition file as defined.
        This is done by setting pre-defined jinja variables for the includes.

        Currently, this is only for the "app" container.

        From syntax (lower, file in repo): apptainer/<container>_<section>_<method>
        Jinja variable (upper): SECTION_<section>_<method>

        Current supported includes:
            apptainer/app_environment
            apptainer/app_post_begin
            apptainer/app_post_pre_make
            apptainer/app_post_pre_install
            apptainer/app_post_post_install
            apptainer/app_post_end
            apptainer/app_test_begin
            apptainer/app_test_end
        """
        if self.args.library != 'app':
            return

        app_name, app_root, _ = Versioner.get_app()
        sections = ['environment', 'post_begin', 'post_pre_make', 'post_pre_install',
                    'post_post_install', 'post_end', 'test_begin', 'test_end']

        for section in sections:
            filename = f'apptainer/app_{section}'
            full_filename = os.path.join(app_root, filename)
            if not os.path.isfile(full_filename):
                continue

            var = f'SECTION_{section.upper()}'
            file_contents = open(full_filename, 'r').read()
            contents = f"# Begin include from '{filename}' in {app_name}\n"
            contents += file_contents
            contents += f"# End include from '{filename}' in {app_name}\n"
            contents = self.add_def_whitespace(contents)

            jinja_data[var] = contents

    def add_definition_vars(self, jinja_data):
        """
        Adds conditional apptainer definition vars to jinja data
        """
        jinja_data['ARCH'] = platform.machine()

        # Set application-related variables
        if self.args.library == 'app':
            app_name, app_root, _ = Versioner.get_app()
            jinja_data['APPLICATION_DIR'] = app_root
            jinja_data['APPLICATION_NAME'] = os.path.basename(app_root)
            jinja_data['BINARY_NAME'] = app_name

        # Set MOOSE_[TOOLS, TEST_TOOLS]_VERSION
        if self.args.library == 'moose-dev':
            # test-tools is deprecated, but leave it here, so this scripts continues
            # to function properly against older hashes.
            for package in ['tools']:
                meta_yaml = os.path.join(MOOSE_DIR, f'conda/{package}/meta.yaml')
                if os.path.exists(meta_yaml):
                    with open(meta_yaml, 'r') as meta_contents:
                        _, version, _, _, _, = Versioner.conda_meta_jinja(meta_contents.read())
                        variable_name = 'MOOSE_'
                        variable_name += package.upper().replace('-', '_')
                        variable_name += '_VERSION'
                        jinja_data[variable_name] = version
        elif self.args.library == 'libmesh':
            package = 'libmesh-vtk'
            meta_yaml = os.path.join(MOOSE_DIR, f'conda/{package}/meta.yaml')
            with open(meta_yaml, 'r') as meta_contents:
                _, _, _, _, meta = Versioner.conda_meta_jinja(meta_contents.read())

            # Jinja returns a list of dictionaries, when variants are involved.
            # Dictionary comprehensions: https://stackoverflow.com/questions/28243504/convert-list-of-dictionaries-into-dict
            # Thankfully, the value of 'var' below will always be the same no matter the variant
            kv_pairs = {k:v for element in meta['source'] for k,v in element.items()}

            for var in ['url', 'sha256', 'vtk_friendly_version']:
                jinja_var = f'vtk_{var}'
                jinja_data[jinja_var] = kv_pairs[var]

        # Set petsc and libmesh versions
        need_versions = {'petsc': {'package': 'petsc', 'submodule': 'petsc'},
                         'libmesh': {'package': 'libmesh', 'submodule': 'libmesh'},
                         'moose-dev': {'package': 'wasp', 'submodule': 'framework/contrib/wasp'}}
        for library, package_info in need_versions.items():
            if library == self.args.library:
                package = package_info['package']
                submodule = package_info['submodule']

                repo_sha = self.git_submodule_sha(MOOSE_DIR, submodule)
                repo_remote = self.git_submodule_remote(MOOSE_DIR, submodule)

                variable_prefix = f'{package}_'.upper()
                jinja_data[variable_prefix + 'GIT_SHA'] = repo_sha
                jinja_data[variable_prefix + 'GIT_REMOTE'] = repo_remote

        # Add include contents, if any
        self.add_definition_includes(jinja_data)

    def _action_exists(self):
        """
        Performs the "exists" action
        """
        uri = self.oras_uri(self.project, self.name, self.tag, project_suffix=self.args.project_suffix)
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

        uri = self.oras_uri(self.project, self.name, self.tag, project_suffix=self.args.project_suffix)
        if not self.oras_exists(uri):
            self.error(f'Container {uri} does not exist')

        args = [] if self.args.pull_args is None else self.args.pull_args.split(' ')
        self.apptainer_pull(self.project, self.name, self.tag, project_suffix=self.args.project_suffix, args=args)

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
        # User provided an alternate dependency
        if self.args.dep:
            self.print(f'Using alternate dependency {self.args.dep}')
            # Provided one but we don't need it
            if not needs_from:
                self.error(f'Library {self.name} does not need a dependency, but --dep was provided')
        # No dependent library needed
        if dep_meta is None:
            if needs_from:
                self.error(f'Library {self.name} needs a dependency, but none found')
        # Dependency library is needed, figure out how to get it
        else:
            apptainer_bootstrap, apptainer_from = None, None
            if not needs_from:
                self.error(f'Definition {self.def_path} missing a templated BootStrap')
            if self.args.dep:
                if self.args.dep.startswith('oras'):
                    apptainer_bootstrap = 'oras'
                    apptainer_from = self.args.dep.replace('oras://', '')
                else:
                    apptainer_bootstrap = 'localimage'
                    apptainer_from = self.args.dep
            else:
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

        # Add a header
        new_definition = self._definition_header(jinja_data) + new_definition

        # Add in a few labels
        new_definition = self._add_definition_labels(new_definition)

        # Add in the environment which contains the version strings
        new_definition = self._add_definition_environment(new_definition)

        # Definition file checks
        container_definition_path = self.container_path(self.name, self.tag, image=False)
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
        if self.args.skip_tests:
            args.append('-T')
        if self.args.no_cleanup:
            args.append('--no-cleanup')
        if self.args.build_args is not None:
            args.extend(self.args.build_args.split(' '))
        container_definition_path = self.container_path(self.name, self.tag, image=False)
        self.apptainer_build(container_definition_path, self.name, self.tag, args=args)

        # Sign if requested
        if self.args.sign is not None:
            self.apptainer_sign(self.name, self.tag, self.args.sign)

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

        uri = self.oras_uri(self.project, self.name, to_tag, project_suffix=self.args.to_project_suffix)
        if self.oras_exists(uri):
            if self.args.overwrite:
                self.warn(f'Overwriting {uri}')
            else:
                self.error(f'Tag {uri} already exists')

        self.apptainer_push(self.project, self.name, from_tag, to_tag, project_suffix=self.args.to_project_suffix)

    def _action_path(self):
        """
        Performs the "path" action
        """
        print(self.container_path(self.name, self.tag))

    def _action_uri(self):
        """
        Performs the "uri" action
        """
        uri = self.oras_uri(self.project, self.name, self.tag, project_suffix=self.args.project_suffix)
        if self.args.check and not self.oras_exists(uri):
            self.error(f'Container {uri} does not exist')
        print(uri)

    def _action_tag(self):
        """
        Performs the "tag" action
        """
        print(self.tag)

    def main(self):
        """
        Runs the specificed CLI action
        """
        getattr(self, f'_action_{self.args.action}')()

if __name__ == '__main__':
    ApptainerGenerator().main()
