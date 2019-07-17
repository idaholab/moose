#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

# This script finds the current repository revision base on the log file
# It currently understands both local git-svn and svn repositories

import subprocess, os, sys, re
from distutils.version import LooseVersion

def shellCommand(command, cwd=None):
    """
    Run a command in the shell.
    We can ignore anything on stderr as that can potentially mess up the output
    of an otherwise successful command.
    """
    with open(os.devnull, 'w') as devnull:
        p = subprocess.Popen(command, shell=True, stdout=subprocess.PIPE, stderr=devnull, cwd=cwd)
        p.wait()
        retcode = p.returncode
        if retcode != 0:
            raise Exception("Exception raised while running the command: " + command + " in directory: " + cwd)

        return p.communicate()[0].decode('utf-8')

class VersionInfo:
    def __init__(self):
        self.hide_signature_string = None
        self.SHA1 = None
        self.is_git_repo = False

    def hideSignature(self):
        """
        Conditionally returns the flag --no-show-signature if the version
        of git is new enough to support that option.
        """
        if self.hide_signature_string == None:
            self.hide_signature_string = ""
            try:
                # Search for a version string in the git version output
                m = re.search(r"(\d+\.\S+)", shellCommand('git --version'))
                if m:
                    gitVersion = m.group(1)
                    if LooseVersion(gitVersion) >= LooseVersion("2.9"):
                        self.hide_signature_string = "--no-show-signature"
            except:
                pass

        return self.hide_signature_string

    def gitSHA1(self, cwd=None):
        if self.SHA1 == None:
            try:
                # The SHA1 should always be available if we have a git repo.
                self.SHA1 = shellCommand('git show ' + self.hideSignature() + ' -s --format=%h', cwd).strip()
                self.is_git_repo = True
            except: # subprocess.CalledProcessError:
                self.SHA1 = None

        return self.SHA1

    def gitTag(self, cwd=None):
        try:
            # The tag should always be available if we have a git repo.
            return shellCommand('git describe --tags', cwd).strip()
        except: # subprocess.CalledProcessError:
            return None


    def gitDate(self, cwd=None):
        try:
            # The date should always be available if we have a git repo.
            return shellCommand('git show ' + self.hideSignature() + ' -s --format=%ci', cwd).split()[0]
        except: # subprocess.CalledProcessError:
            return None


    def gitVersionString(self, cwd=None):
        SHA1 = self.gitSHA1(cwd)
        date = self.gitDate(cwd)

        if not SHA1 or not date:
            # Can happen if we aren't in a valid repo
            return None

        # The tag check will always succeed if the repo starts with a v0.0 tag. To find only tags that
        # were part of the first parent, use --first-parent.
        tag = ''
        try:
            description = shellCommand('git describe --tags --long --match "v[0-9]*"', cwd).rsplit('-',2)
            tag = description[0] + ', '
            commitsSinceTag = description[1]
            if commitsSinceTag != '0':
                tag = 'derived from ' + tag
        except: # subprocess.CalledProcessError:
            pass
        return tag + "git commit " + SHA1 + " on " + date


    def gitSvnVersionString(self, cwd=None):
        try:
            date = shellCommand('git show -s --format=%ci', cwd).split()[0]
            revision = shellCommand('git svn find-rev $(git log --max-count 1 --pretty=format:%H)', cwd).strip()
            if len(revision) > 0 and len(revision) < 10:
                return 'svn revision ' + revision + " on " + date
        except: # subprocess.CalledProcessError:
            pass
        return None


    def svnVersionString(self, cwd=None):
        try:
            revisionString = shellCommand('svnversion .', cwd)
            matchingRevision = re.search(r'\d+', revisionString)
            if matchingRevision is not None:
                return 'svn revision ' + matchingRevision.group(0)
        except: # subprocess.CalledProcessError:
            pass
        return None


    def repoVersionString(self, cwd=None):
        version = self.gitVersionString(cwd)
        if version is None:
            version = self.svnVersionString(cwd)
        if version is None:
            version = self.gitSvnVersionString(cwd)
        if version is None:
            version = "unknown"
        return version


    def writeRevision(self, repo_location, app_name, revision_header):
        # Use all caps for app name (by convention).
        app_def_name = app_name.upper()
        app_def_name = re.sub('[^a-zA-Z0-9]', '', app_def_name)
        app_definition = app_def_name + '_REVISION'

        app_revision = self.repoVersionString(repo_location)

        # Version is the tag. If empty, we use the sha1 hash
        app_version = self.gitTag(repo_location)
        if app_version is None:
            app_version = self.gitSHA1(repo_location)
            if app_version is None:
                app_version = "unknown"

        # see if the revision is different
        revision_changed = False
        if os.path.exists(revision_header):
            f = open(revision_header, "r")
            buffer = f.read()

            m =  re.search(re.escape(app_definition) + r' "([^"]*)"',buffer)
            if m is not None and m.group(1) != app_revision:
                revision_changed = True

            if m is None:
                # If the above RE doesn't match anything then we have
                # a bad _REVISION define and we need to rewrite
                # out a new file.
                revision_changed = True

            f.close()
        else:
            # Count it as changed if the header does not exist.
            revision_changed = True

        if revision_changed:
            revision_dir = os.path.dirname(revision_header)
            try:
                os.stat(revision_dir)
            except:
                os.mkdir(revision_dir)

            f = open(revision_header, "w")
            f.write( '/* THIS FILE IS AUTOGENERATED - DO NOT EDIT */\n' \
                     '\n'
                     '#pragma once\n'
                     '\n'
                     '#define ' + app_definition + ' "' + app_revision + '"\n'
                     '#define ' + app_def_name + '_VERSION "' + app_version + '"\n')
            f.close()
        elif self.is_git_repo:
            # Nothing has changed, but if we don't do anything the revision file
            # will still appear to be out of date every time we re-run make.
            # We could touch the revision file, but that might spawn off another big
            # build. Instead we will touch the repository files that triggered this
            # script instead (e.g. .git/HEAD .git/index).
            # Hopefully this doesn't cause the Universe to unravel.
            cdup = shellCommand('git rev-parse --show-cdup', repo_location).strip()

            timestamps_to_modify = []
            timestamps_to_modify.append(os.path.join(cdup, '.git', 'HEAD'))
            timestamps_to_modify.append(os.path.join(cdup, '.git', 'index'))

            for touch_file in timestamps_to_modify:
                if os.path.isfile(os.path.join(repo_location, touch_file)):
                    shellCommand("touch -r " + revision_header + " " + touch_file, repo_location)


# Entry point
if len(sys.argv) == 4:
    repo_location = sys.argv[1]
    header_file = sys.argv[2]
    app_name = sys.argv[3]

    version_info = VersionInfo()
    version_info.writeRevision(repo_location, app_name, header_file)
