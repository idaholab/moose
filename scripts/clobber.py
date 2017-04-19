#!/usr/bin/env python
"""
Script to cleanup compile artifacts.
This can be called from a top level application so
we ignore the following directories:
    moose (ignore a possible MOOSE submodule)
    .git  (don't accidentally delete any of git's metadata)
    .svn  (don't accidentally delete any of svn's metadata)
"""
import os
import argparse
import shutil

def verbose_print(s, verbose=False):
    if verbose:
        print(s)

def ignore_dir(root, subdirs, to_ignore, verbose):
    """
    Ignores a directory by removing it from the subdirs list
    """
    if to_ignore in subdirs:
        verbose_print("Ignoring %s" % os.path.join(root, to_ignore), verbose)
        subdirs.remove(to_ignore)

def remove_dir(root, subdirs, to_remove, verbose):
    """
    Removes a directory in the filesystem.
    """
    if to_remove in subdirs:
        dir_path = os.path.join(root, to_remove)
        verbose_print("Removing directory %s" % dir_path, verbose)
        subdirs.remove(to_remove)
        try:
            shutil.rmtree(dir_path)
        except Exception as e:
            print("Failed to remove path: %s\nError: %s" % (dir_path, e))

def clobber(top_dir, verbose=False):
    """
    Removes compilation related files and directories.
    """
    verbose_print("Clobbering in %s" % top_dir, verbose)
    remove_ext = ("~",
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
            ".plist",
            ".gcda",
            ".gcno",
            ".gcov",
            ".gch",
            )

    for root, subdirs, files in os.walk(top_dir, topdown=True):
        ignore_dir(root, subdirs, "moose", verbose)
        ignore_dir(root, subdirs, ".git", verbose)
        ignore_dir(root, subdirs, ".svn", verbose)

        remove_dir(root, subdirs, ".libs", verbose)
        remove_dir(root, subdirs, ".jitcache", verbose)

        for f in files:
            if f.endswith(remove_ext) or ".so" in f:
                fpath = os.path.join(root, f)
                verbose_print("Removing file %s" % fpath, verbose)
                try:
                    os.unlink(fpath)
                except Exception as e:
                    print("Failed to remove file: %s\nError: %s" % (fpath, e))

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Remove compile artifacts')
    parser.add_argument('-v', action='store_true', dest='verbose', help='Print what is happening')
    parser.add_argument('top_directory', help='Top directory to start with')
    parsed = parser.parse_args()
    clobber(parsed.top_directory, parsed.verbose)
