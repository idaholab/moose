#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html
import os
import re
import subprocess
import logging
import collections
import mooseutils

def git_is_config(key, value, working_dir=os.getcwd()):
    """
    Return True if the supplied *key* from the git config matches the supplied *value*.

    The command runs `git config --get <key>` and compares the result with *value*.
    """
    out = mooseutils.check_output(['git', 'config', '--get', key], cwd=working_dir).strip()
    return out == value

def git_is_branch(name, working_dir=os.getcwd()):
    """
    Return True if the supplied *name* matches the current git branch.

    If the current location is not a repository, False is returned.
    """
    if git_is_repo(working_dir):
        out = mooseutils.check_output(['git', 'rev-parse', '--abbrev-ref', 'HEAD'], cwd=working_dir).strip()
        return out == name
    return False

def git_is_repo(working_dir=os.getcwd()):
    """
    Return true if the repository is a git repo.
    """
    out = mooseutils.check_output(['git', 'rev-parse', '--is-inside-work-tree'], check=False,
                       stderr=subprocess.PIPE, cwd=working_dir).strip(' \n')
    return out.lower() == 'true'

def git_commit(working_dir=os.getcwd()):
    """
    Return the current SHA from git.
    """
    out = mooseutils.check_output(['git', 'rev-parse', 'HEAD'], cwd=working_dir)
    return out.strip(' \n')

def git_commit_message(sha, working_dir=os.getcwd()):
    """
    Return the the commit message for the supplied SHA
    """
    out = mooseutils.check_output(['git', 'show', '-s', '--format=%B', sha], cwd=working_dir)
    return out.strip(' \n')

def git_merge_commits(working_dir=os.getcwd()):
    """
    Return the current SHAs for a merge.
    """
    out = mooseutils.check_output(['git', 'log', '-1', '--merges', '--pretty=format:%P'], cwd=working_dir)
    return out.strip(' \n').split(' ')

def git_ls_files(working_dir=os.getcwd(), recurse_submodules=False, exclude=None):
    """
    Return a list of files via 'git ls-files'.
    """
    cmd = ['git', 'ls-files']
    if recurse_submodules:
        cmd.append('--recurse-submodules')
    if exclude is not None:
        cmd += ['--exclude', exclude]
    out = set()
    for fname in mooseutils.check_output(cmd, cwd=working_dir).split('\n'):
        out.add(os.path.abspath(os.path.join(working_dir, fname)))
    return out

def git_root_dir(working_dir=os.getcwd()):
    """
    Return the top-level git directory by running 'git rev-parse --show-toplevel'.
    """
    try:
        return mooseutils.check_output(['git', 'rev-parse', '--show-toplevel'],
                            cwd=working_dir, stderr=subprocess.STDOUT).strip('\n')
    except subprocess.CalledProcessError:
        print("The supplied directory is not a git repository: {}".format(working_dir))
    except OSError:
        print("The supplied directory does not exist: {}".format(working_dir))

def git_submodule_info(working_dir=os.getcwd(), *args):
    """
    Return the status of each of the git submodule(s).

    NOTE: It is possible that the 'git submodule status' command fails if one of the additional args
          is '--recursive' and there are broken or empty (null) submodules.
    """
    out = dict()
    result = mooseutils.check_output(['git', 'submodule', 'status', *args], cwd=working_dir)
    regex = re.compile(r'(?P<status>[\s\-\+U])(?P<sha1>[a-f0-9]{40})\s(?P<name>.*?)\s(?P<refs>(\(.*\))?)')
    for match in regex.finditer(result):
        out[match.group('name')] = (match.group('status'), match.group('sha1'), match.group('refs'))
    return out

def git_init_submodule(path, working_dir=os.getcwd()):
    """
    Check if given path is a submodule and initialize it (unless it already has been) if so.
    """
    status = git_submodule_info(working_dir)
    for submodule, status in status.items():
        if (submodule == path) and ((status[0] == '-') or ('(null)' in status[2])):
            subprocess.call(['git', 'submodule', 'update', '--init', path], cwd=working_dir)
            break

def git_version():
    """
    Return the version number as a tuple (major, minor, patch)
    """
    out = mooseutils.check_output(['git', '--version'], encoding='utf-8')
    match = re.search(r'(?P<major>\d+)\.(?P<minor>\d+)\.(?P<patch>\d+)', out)
    if match is None:
        raise SystemError("git --version failed to return correctly formatted version number")
    return (int(match.group('major')), int(match.group('minor')), int(match.group('patch')))

def git_authors(loc=None):
    """
    Return a complete list of authors for the given location.

    Inputs:
      loc: File/directory to consider
    """
    if not os.path.exists(loc):
        raise OSError("The supplied location must be a file or directory: {}".format(loc))
    loc = loc or os.getcwd()
    out = mooseutils.check_output(['git', 'shortlog', '-n', '-c', '-s', '--', loc], encoding='utf-8')
    names = list()
    for match in re.finditer(r'^\s*\d+\s*(?P<name>.*?)$', out, flags=re.MULTILINE):
        names.append(match.group('name'))
    return names

def git_lines(filename, blank=False):
    """
    Return the number of lines per author for the given filename
    Inputs:
      filename: Filename to consider
      blank[bool]: Include/exclude blank lines
    """
    if not os.path.isfile(filename):
        raise OSError("File does not exist: {}".format(filename))
    regex = re.compile(r'^.*?\((?P<name>.*?)\s+\d{4}-\d{2}-\d{2}.*?\)\s+(?P<content>.*?)$', flags=re.MULTILINE)
    counts = collections.defaultdict(int)
    blame = mooseutils.check_output(['git', 'blame', '--', filename], encoding='utf-8')
    for line in blame.splitlines():
        match = regex.search(line)
        if blank or len(match.group('content')) > 0:
            counts[match.group('name')] += 1
    return counts

def git_committers(loc=os.getcwd(), *args):
    """
    Return the number of commits per author given a location

    Inputs:
       loc[str]: Filename/directory to consider
       args: Appended to 'git shortlog -s' command (e.g., '--merges', '--no-merges', etc.)
    """
    if not os.path.exists(loc):
        raise OSError("The supplied location must be a file or directory: {}".format(loc))
    cmd = ['git', 'shortlog', '-s']
    cmd += args
    cmd += ['--', loc]
    committers = mooseutils.check_output(cmd, encoding='utf-8')
    counts = collections.defaultdict(int)
    for line in committers.splitlines():
        items = line.split("\t", 1)
        counts[items[1]] = int(items[0])
    return counts

def git_localpath(filename):
    """
    Return the path from the root of the repository.
    """
    root = git_root_dir(os.path.dirname(filename))
    return os.path.relpath(filename, root)

def git_repo(loc=os.getcwd(), remotes=['upstream', 'origin']):
    """
    Return URL to repository based on remotes
    """
    if not os.path.isdir(loc):
        raise OSError("The supplied location must be a directory: {}".format(loc))

    lookup = dict()
    for remote in mooseutils.check_output(['git', 'remote', '-v'], encoding='utf-8', cwd=loc).strip(' \n').split('\n'):
        name, addr = remote.split(maxsplit=1)
        lookup[name] = addr

    for remote in remotes:
        address = lookup.get(remote, None)
        if address is not None:
            break

    if address is None:
        raise OSError("Unable to locate a remote with the name(s): {}".format(', '.join(remotes)))

    if address.startswith('git'):
        match = re.match(r'git@(?P<host>.*?):(?P<site>.*?)\.git', address)
        address = 'https://{}/{}'.format(match.group('host'), match.group('site'))
    return address

def git_remotes(working_dir=None):
    """
    Return URL to name remotes.
    """
    if working_dir is None: working_dir = os.getcwd()
    lookup = dict()
    for remote in mooseutils.check_output(['git', 'remote', '-v'], encoding='utf-8', cwd=working_dir).strip(' \n').split('\n'):
        name, addr = remote.split(maxsplit=1)
        lookup[addr] = name
    return lookup

def git_add_and_fetch_remote(url, name, branch, working_dir=None):
    """
    Add then fetch a remote with *name* and remote location *url*.
    """
    mooseutils.check_output(['git', 'remote', 'add', name, url], cwd=working_dir, check=False)
    mooseutils.check_output(['git', 'fetch', name, branch], cwd=working_dir, check=True)

def git_fetch_remote(name, branch, working_dir=None):
    """
    Add then fetch a remote with *name* and remote location *url*.
    """
    mooseutils.check_output(['git', 'fetch', name, branch], cwd=working_dir, check=True)
