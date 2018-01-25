#!/usr/bin/env python
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

def shellCommand( command, cwd=None ):
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
            raise Exception()

        return p.communicate()[0]

def hideSignature():
    """
    Conditionally returns the flag --no-show-signature if the version
    of git is new enough to support that option.
    """
    try:
        # Search for a version string in the git version output
        m = re.search(r"(\d+\.\S+)", shellCommand( 'git --version' ))
        if m:
            gitVersion = m.group(1)
            if LooseVersion(gitVersion) >= LooseVersion("2.9"):
                return "--no-show-signature"
    except:
        pass

    return ""


def gitSHA1( cwd=None ):
    try:
        # The SHA1 should always be available if we have a git repo.
        return shellCommand( 'git show ' + hideSignature() + ' -s --format=%h', cwd ).strip()
    except: # subprocess.CalledProcessError:
        return None


def gitTag( cwd=None ):
    try:
        # The tag should always be available if we have a git repo.
        return shellCommand( 'git describe --tags', cwd).strip()
    except: # subprocess.CalledProcessError:
        return None


def gitDate( cwd=None ):
    try:
        # The date should always be available if we have a git repo.
        return shellCommand( 'git show ' + hideSignature() + ' -s --format=%ci', cwd ).split()[0]
    except: # subprocess.CalledProcessError:
        return None


def gitVersionString( cwd=None ):
    SHA1 = gitSHA1( cwd )
    date = gitDate( cwd )

    if not SHA1 or not date:
        # Can happen if we aren't in a valid repo
        return None

    # The tag check will always succeed if the repo starts with a v0.0 tag. To find only tags that
    # were part of the first parent, use --first-parent.
    tag = ''
    try:
        description = shellCommand( 'git describe --tags --long --match "v[0-9]*"', cwd ).rsplit('-',2)
        tag = description[0] + ', '
        commitsSinceTag = description[1]
        if commitsSinceTag != '0':
            tag = 'derived from ' + tag
    except: # subprocess.CalledProcessError:
        pass
    return tag + "git commit " + SHA1 + " on " + date


def gitSvnVersionString( cwd=None ):
    try:
        date = shellCommand( 'git show -s --format=%ci', cwd ).split()[0]
        revision = shellCommand( 'git svn find-rev $(git log --max-count 1 --pretty=format:%H)', cwd ).strip()
        if len( revision ) > 0 and len( revision ) < 10:
            return 'svn revision ' + revision + " on " + date
    except: # subprocess.CalledProcessError:
        pass
    return None


def svnVersionString( cwd=None ):
    try:
        revisionString = shellCommand( 'svnversion .', cwd )
        matchingRevision = re.search( r'\d+', revisionString )
        if matchingRevision is not None:
            return 'svn revision ' + matchingRevision.group(0)
    except: # subprocess.CalledProcessError:
        pass
    return None


def repoVersionString( cwd=None ):
    version = svnVersionString( cwd )
    if version is None:
        version = gitSvnVersionString( cwd )
    if version is None:
        version = gitVersionString( cwd )
    if version is None:
        version = "unknown"
    return version


def writeRevision( repo_location, app_name, revision_header ):
    # Use all caps for app name (by convention).
    app_def_name = app_name.upper()
    app_def_name = re.sub( '[^a-zA-Z0-9]', '', app_def_name )
    app_definition = app_def_name + '_REVISION'

    app_revision = repoVersionString( repo_location )

    # Version is the tag. If empty, we use the sha1 hash
    app_version = gitTag(repo_location)
    if app_version is None:
        app_version = gitSHA1(repo_location)
        if app_version is None:
            app_version = "unknown"

    # see if the revision is different
    revision_changed = False
    if os.path.exists(revision_header):
        f = open(revision_header, "r")
        buffer = f.read()

        m =  re.search( re.escape( app_definition) + r' "([^"]*)"',buffer )
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
                 '#ifndef ' + app_definition + '_H\n'
                 '#define ' + app_definition + '_H\n'
                 '\n'
                 '#define ' + app_definition + ' "' + app_revision + '"\n'
                 '#define ' + app_def_name + '_VERSION "' + app_version + '"\n'
                 '\n'
                 '#endif // ' + app_definition + '_H\n')
        f.close()

# Entry point
if len(sys.argv) == 4:
    repo_location = sys.argv[1]
    header_file = sys.argv[2]
    app_name = sys.argv[3]

    writeRevision( repo_location, app_name, header_file )
