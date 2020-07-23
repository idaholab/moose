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
from .mooseutils import check_output

def is_git_repo(working_dir=os.getcwd()):
    """
    Return true if the repository is a git repo.
    """
    out = check_output(['git', 'rev-parse', '--is-inside-work-tree'], check=False,
                       stderr=subprocess.PIPE, cwd=working_dir).strip(' \n')
    return out.lower() == 'true'

def git_commit(working_dir=os.getcwd()):
    """
    Return the current SHA from git.
    """
    out = check_output(['git', 'rev-parse', 'HEAD'], cwd=working_dir)
    return out.strip(' \n')

def git_commit_message(sha, working_dir=os.getcwd()):
    """
    Return the the commit message for the supplied SHA
    """
    out = check_output(['git', 'show', '-s', '--format=%B', sha], cwd=working_dir)
    return out.strip(' \n')

def git_merge_commits(working_dir=os.getcwd()):
    """
    Return the current SHAs for a merge.
    """
    out = check_output(['git', 'log', '-1', '--merges', '--pretty=format:%P'], cwd=working_dir)
    return out.strip(' \n').split(' ')

def git_ls_files(working_dir=os.getcwd()):
    """
    Return a list of files via 'git ls-files'.
    """
    out = set()
    for fname in check_output(['git', 'ls-files'], cwd=working_dir).split('\n'):
        out.add(os.path.abspath(os.path.join(working_dir, fname)))
    return out

def git_root_dir(working_dir=os.getcwd()):
    """
    Return the top-level git directory by running 'git rev-parse --show-toplevel'.
    """
    try:
        return check_output(['git', 'rev-parse', '--show-toplevel'],
                            cwd=working_dir, stderr=subprocess.STDOUT).strip('\n')
    except subprocess.CalledProcessError:
        print("The supplied directory is not a git repository: {}".format(working_dir))
    except OSError:
        print("The supplied directory does not exist: {}".format(working_dir))

def git_submodule_status(working_dir=os.getcwd()):
    """
    Return the status of each of the git submodule(s).
    """
    out = dict()
    result = check_output(['git', 'submodule', 'status'], cwd=working_dir)
    regex = re.compile(r'(?P<status>[\s\-\+U])(?P<sha1>[a-f0-9]{40})\s(?P<name>.*?)\s')
    for match in regex.finditer(result):
        out[match.group('name')] = match.group('status')
    return out

def git_init_submodule(path, working_dir=os.getcwd()):
    """Check submodule for given in path"""
    status = git_submodule_status(working_dir)
    for submodule, status in status.items():
        if (submodule == path) and (status == '-'):
            subprocess.call(['git', 'submodule', 'update', '--init', path], cwd=working_dir)
            break
