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
import shutil
import unittest
import tempfile
import mooseutils

class Test(unittest.TestCase):
    def testIsGitRepo(self):
        loc = os.path.dirname(__file__)
        self.assertTrue(mooseutils.is_git_repo(loc))

        loc = tempfile.mkdtemp()
        self.assertFalse(mooseutils.is_git_repo(loc))
        os.rmdir(loc)

    @unittest.skipIf(not mooseutils.is_git_repo(), "Not a Git repository")
    def testGitCommit(self):
        c = mooseutils.git_commit()
        self.assertEqual(len(c), 40)

    @unittest.skipIf(not mooseutils.is_git_repo(), "Not a Git repository")
    def testGitCommitMessage(self):
        c = 'b863a496dbf1449853be6978c8ac1a9c242d389b' # beautiful commit
        msg = mooseutils.git_commit_message(c)
        self.assertIn('The name is, just so long', msg)

    @unittest.skipIf(not mooseutils.is_git_repo(), "Not a Git repository")
    def testGitMergeCommits(self):
        merges = mooseutils.git_merge_commits()
        self.assertEqual(len(merges[0]), 40)

    @unittest.skipIf(not mooseutils.is_git_repo(), "Not a Git repository")
    def testGitLsFiles(self):
        files = mooseutils.git_ls_files()
        self.assertIn(os.path.abspath(__file__), files)

    @unittest.skipIf(not mooseutils.is_git_repo(), "Not a Git repository")
    def testGitRootDir(self):
        root = mooseutils.git_root_dir()
        self.assertEqual(root, os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..', '..')))

if __name__ == '__main__':
    unittest.main(verbosity=2, buffer=True)
