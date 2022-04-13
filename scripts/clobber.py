#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import os
import argparse
import shutil

class Clobber(object):
    """
    Class to cleanup compile artifacts.
    This does not honor the METHOD environment variable.
    It removes compile artifacts for all of them.
    This can be called from a top level application so
    we ignore the following directories:
        moose (ignore a possible MOOSE submodule)
        .git  (don't accidentally delete any of git's metadata)
        .svn  (don't accidentally delete any of svn's metadata)
    """
    def __init__(self, top_dir, verbose, dry_run):
        self.top_dir = top_dir
        self.verbose = verbose
        self.dry_run = dry_run

    def message(self, msg):
        if self.verbose:
            if self.dry_run:
                print("DRY RUN: " + msg)
            else:
                print(msg)

    def ignore_dir(self, root, subdirs, to_ignore):
        """
        Ignores a directory by removing it from the subdirs list
        """
        if to_ignore in subdirs:
            self.message("Ignoring %s" % os.path.join(root, to_ignore))
            subdirs.remove(to_ignore)

    def remove_dir(self, root, subdirs, to_remove):
        """
        Removes a directory in the filesystem.
        """
        if to_remove in subdirs:
            dir_path = os.path.join(root, to_remove)
            self.message("Removing directory %s" % dir_path)
            subdirs.remove(to_remove)
            if not self.dry_run:
                try:
                    shutil.rmtree(dir_path)
                except Exception as e:
                    print("Failed to remove path: %s\nError: %s" % (dir_path, e))

    def clobber(self):
        """
        Walks the directories and removes unwanted files and directories.
        """
        self.message("Clobbering in %s" % self.top_dir)
        remove_file_ext = ("~",
                ".o",
                ".lo",
                ".la",
                ".dylib",
                ".a",
                "-opt",
                "-dbg",
                "-oprof",
                "-devel",
                ".d",
                ".pyc",
                ".plugin",
                ".mod",
                ".gcda",
                ".gcno",
                ".gcov",
                ".gch",
                ".so",
                )

        for root, subdirs, files in os.walk(self.top_dir, topdown=True):
            self.ignore_dir(root, subdirs, "moose")
            self.ignore_dir(root, subdirs, ".git")
            self.ignore_dir(root, subdirs, ".svn")

            self.remove_dir(root, subdirs, ".libs")
            self.remove_dir(root, subdirs, ".jitcache")

            for f in files:
                if f.endswith(remove_file_ext):
                    fpath = os.path.join(root, f)
                    self.message("Removing file %s" % fpath)
                    if not self.dry_run:
                        try:
                            os.unlink(fpath)
                        except Exception as e:
                            print("Failed to remove file: %s\nError: %s" % (fpath, e))

            for d in subdirs[:]:
                if d.endswith(".dSYM"):
                    self.remove_dir(root, subdirs, d)

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Remove compile artifacts.')
    parser.add_argument('-v', action='store_true', dest='verbose', help='Print what is happening.')
    parser.add_argument('-n', action='store_true', dest='dry_run', help="Dry run. Don't actually remove anything.")
    parser.add_argument('top_directory', help='Top directory to start with.')
    parsed = parser.parse_args()
    c = Clobber(parsed.top_directory, parsed.verbose, parsed.dry_run)
    c.clobber()
