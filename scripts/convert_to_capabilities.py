#!/usr/bin/env python3
import argparse
import os
import subprocess
import sys
import traceback

MOOSE_DIR = os.environ.get('MOOSE_DIR', os.path.abspath(os.path.join(os.path.dirname(os.path.realpath(__file__)), '..')))
sys.path.append(os.path.join(MOOSE_DIR, 'python'))
import pyhit

class SpecModifier:
  """
  Helper class that modifies a single test spec to use capabilities
  """
  def __init__(self, file: str):
    """
    Args:
      file: Path to the file
    """
    # Path to the file
    self.file = file
    # Whether or not we've modified the file
    self.modified = False

  def recurse(self, node: pyhit.Node):
    """
    Recursively find a single test, try to change it,
    and modify all of its children
    """
    for child in node:
      if child.get('type') is not None:
        self.modified = self.modify_test(child)
      else:
        self.recurse(child)

  @staticmethod
  def modify_test(node: pyhit.Node) -> bool:
    """
    Modifes a test, changing old options to capabilities

    Args:
      node: The node that represents
    Returns:
      bool: Whether or not it was modified
    """
    assert node.get('type') is not None

    # Options that we've converted to capabilities
    new_capabilities = []

    def fix_param(param: str, get_capabilities):
      value = node.get(param)
      if value is None:
        return
      capabilities = get_capabilities(value)
      if not capabilities:
        return
      if ' ' in capabilities:
        capabilities = f'({capabilities})'
      new_capabilities.append(capabilities)
      node.removeParam(param)

    # min_ad_size
    do_min_ad_size = lambda value: f'ad_size>={value}'
    fix_param('min_ad_size', do_min_ad_size)
    # max_ad_size
    do_max_ad_size = lambda value: f'ad_size<={value}'
    fix_param('max_ad_size', do_max_ad_size)
    # *_version
    versioned_params = ['petsc', 'slepc', 'exodus', 'vtk', 'libtorch']
    for param in versioned_params:
      param_name = f'{param}_version'
      def do_versioned_param(value):
        conditions = []
        value_split = value.split()
        for entry in value_split:
          if entry == '&&':
            conditions.append('&')
          elif entry == '||':
            conditions.append('|')
          else:
            conditions.append(f'{param}{entry}')
        return ' '.join(conditions)
      fix_param(param_name, do_versioned_param)
    # string params, implicit or, negation allowed
    string_params = ['compiler', 'method', 'platform']
    for param in string_params:
      def do_string_param(value):
        conditions = []
        for split_value in value.split(' '):
          prefix = ''
          if split_value[0] == '!':
            prefix = '!'
            split_value = split_value[1:]
          conditions.append(f'{param}{prefix}={split_value.lower()}')
        return ' | '.join(conditions)
      fix_param(param, do_string_param)
    # boolean params
    bool_params = ['unique_id', 'tecplot', 'vtk', 'petsc_debug', 'superlu',
                   'mumps', 'strumpack', 'slepc',
                   'parmetis', 'chaco', 'party', 'ptscotch', 'libpng',
                   'libtorch', 'mfem']
    for param in bool_params:
      def do_bool_param(value):
        bool_value = None
        if isinstance(value, bool):
          bool_value = value
        elif isinstance(value, str):
          bool_value = value.strip().lower() == 'true'
        else:
          return None
        prefix = '!' if not bool_value else ''
        return f'{prefix}{param}'
      fix_param(param, do_bool_param)
    # numeric params
    numeric_params = ['dof_id_bytes']
    for param in numeric_params:
      do_numeric_param = lambda value: f'{param}={value}'
      fix_param(param, do_numeric_param)
    # fparse_jit
    do_fparser_jit = lambda value: 'fparser=jit'
    fix_param('fparser_jit', do_fparser_jit)
    # required_applications
    def do_required_applications(value):
      conditions = []
      for value_split in value.split(' '):
        conditions.append(value_split.lower())
      return ' & '.join(conditions)
    fix_param('required_applications', do_required_applications)

    # Nothing to do
    if not new_capabilities:
      return False

    # Get current value to extend if needed
    capabilities = node.get('capabilities', '')
    if capabilities:
      capabilities += ' & '
    # Add new capabilities
    capabilities += ' & '.join(new_capabilities)
    # Set capabilities param
    node['capabilities'] = f"'{capabilities}'"

    # Did something
    return True

  def run(self, dry_run: bool) -> bool:
    """
    Main runner to modify a spec.

    Returns whether or not the spec was modified.
    """
    root = pyhit.load(self.file)

    # Find [Tests] node
    tests_node = None
    for child in root:
      if child.name == 'Tests':
        tests_node = child
        break

    # Nothing to do
    if tests_node is None:
      return False

    # Recurse, modifying each test
    self.recurse(tests_node)
    # Nothing changed
    if not self.modified:
      return False

    # Overwrite changes
    if not dry_run:
      pyhit.write(self.file, root)
    return True

def parse_args():
  parser = argparse.ArgumentParser(description='Converts test specs to capabilities')
  parser.add_argument('root_dir', type=str, help='Root directory to start the search')
  parser.add_argument('--spec-file', type=str, default='tests',
                      help='Name of the spec file (default: tests)')
  parser.add_argument('--check', action='store_true', default=False,
                      help='Perform a check; do not make changes and exit nonzero if changes are needed')
  return parser.parse_args()

def main():
  args = parse_args()
  root_dir = os.path.abspath(args.root_dir)
  check = args.check

  if not os.path.exists(root_dir):
    print(f'ERROR: Root directory {root_dir} does not exist')
    sys.exit(1)

  skip_dirs = []

  # See if we're in a git repository so we can skip checking submodules
  git_root = None
  try:
    git_root = subprocess.check_output(['git', 'rev-parse', '--show-toplevel'],
                                        cwd=root_dir, text=True).strip()
  except:
    pass

  # In a git repository, skip checking .git directory and all submodules
  if git_root:
    skip_dirs.append(os.path.join(git_root, '.git'))
    gitmodules = subprocess.check_output(['git', 'config', '--file', '.gitmodules',
                                          '--name-only', '--get-regexp', 'path'],
                                          cwd=git_root, text=True).strip()
    for entry in gitmodules.splitlines():
      skip_dirs.append(os.path.join(git_root, ''.join(entry.split('.')[1:-1])))

  test_specs = []
  for dirpath, _, filenames in os.walk(root_dir, followlinks=True):
    # Directory is skipped
    if [d for d in skip_dirs if dirpath.startswith(d)]:
      continue

    for filename in filenames:
      path = os.path.abspath(os.path.join(dirpath, filename))
      if path.endswith(f'/{args.spec_file}'):
        test_specs.append(path)

  print(f'Found {len(test_specs)} test specs in {os.path.abspath(args.root_dir)}')
  num_modified = 0
  num_failed = 0
  for path in test_specs:
    path = os.path.relpath(path)
    modified = False
    try:
      modified = SpecModifier(path).run(check)
    except:
      print(f'FAILED {path}')
      print(traceback.format_exc())
      num_failed += 1
    if modified:
      print('CHANGES REQUIRED' if check else 'MODIFIED', path)
      num_modified += 1
  if check:
    print(f'{num_modified} test specs require changes')
    if num_modified:
      sys.exit(1)
  else:
    print(f'Modified {num_modified} test specs, {num_failed} failed modifications')

if __name__ == '__main__':
  main()
