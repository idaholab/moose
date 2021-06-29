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
import mock
import tempfile
import subprocess
import platform
import mooseutils

#from mooseutils.gitutils import git_submodule_info

class Test(unittest.TestCase):
    def testIsGitRepo(self):
        loc = os.path.dirname(__file__)
        self.assertTrue(mooseutils.git_is_repo(loc))

        loc = tempfile.mkdtemp()
        self.assertFalse(mooseutils.git_is_repo(loc))
        os.rmdir(loc)

    @unittest.skipIf(not mooseutils.git_is_repo(), "Not a Git repository")
    def testGitCommit(self):
        c = mooseutils.git_commit()
        self.assertEqual(len(c), 40)

    @unittest.skipIf(not mooseutils.git_is_repo(), "Not a Git repository")
    def testGitCommitMessage(self):
        c = 'b863a496dbf1449853be6978c8ac1a9c242d389b' # beautiful commit
        msg = mooseutils.git_commit_message(c)
        self.assertIn('The name is, just so long', msg)

    @unittest.skipIf(not mooseutils.git_is_repo(), "Not a Git repository")
    def testGitMergeCommits(self):
        merges = mooseutils.git_merge_commits()
        self.assertEqual(len(merges[0]), 40)

    @unittest.skipIf(not mooseutils.git_is_repo(), "Not a Git repository")
    def testGitLsFiles(self):
        files = mooseutils.git_ls_files()
        self.assertIn(os.path.abspath(__file__), files)

    @unittest.skipIf(not mooseutils.git_is_repo(), "Not a Git repository")
    def testGitRootDir(self):
        root = mooseutils.git_root_dir()
        self.assertEqual(root, os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..', '..')))

    @unittest.skipIf(not mooseutils.git_is_repo(), "Not a Git repository")
    def testGitSubmoduleInfo(self):
        root = mooseutils.git_root_dir()
        status = mooseutils.git_submodule_info(root)
        self.assertIn('large_media', status)
        self.assertIn('libmesh', status)
        self.assertIn('petsc', status)
        self.assertEqual(len(status['large_media']), 3)
        self.assertEqual(len(status['libmesh']), 3)
        self.assertEqual(len(status['petsc']), 3)

    @mock.patch('subprocess.call')
    @mock.patch('mooseutils.gitutils.git_submodule_info')
    @unittest.skipIf(not mooseutils.git_is_repo(), "Not a Git repository")
    def testGitInitSubmodule(self, status_func, call_func):
        status_func.return_value = {'test':'-'}

        root = mooseutils.git_root_dir()
        mooseutils.git_init_submodule('test', root)

        status_func.assert_called_with(root)
        call_func.assert_called_with(['git', 'submodule', 'update', '--init', 'test'], cwd=root)

    def testGitVersion(self):
        ver = mooseutils.git_version()
        self.assertEqual(len(ver), 3)
        self.assertIsInstance(ver[0], int)
        self.assertIsInstance(ver[1], int)
        self.assertIsInstance(ver[2], int)

    @mock.patch('re.search')
    def testGitVersion2(self, re_func):
        re_func.return_value = None
        with self.assertRaises(SystemError):
            ver = mooseutils.git_version()

    @unittest.skip("Fails on CIVET and I can't reproduce it")
    def testGitAuthors(self):
        names = mooseutils.git_authors(mooseutils.__file__)
        self.assertIn('Andrew E. Slaughter', names)

        with self.assertRaises(OSError) as e:
            mooseutils.git_authors('wrong')

    @unittest.skip("Fails on CIVET and I can't reproduce it")
    def testCommitters(self):
        names = mooseutils.git_committers(mooseutils.__file__)
        self.assertIn('Andrew E. Slaughter', names)
        with self.assertRaises(OSError) as e:
            mooseutils.git_authors('wrong')

        names = mooseutils.git_committers(mooseutils.__file__, '--merges')
        self.assertIn('Logan Harbour', names)

    def testGitLines(self):
        with open(__file__, 'r') as fid:
            lines = fid.readlines()

        n_with_blank = len(lines)
        n_no_blank = n_with_blank - len([l for l in lines if not l.strip()])

        counts = mooseutils.git_lines(__file__)
        self.assertIn('Andrew E. Slaughter', counts)
        self.assertTrue(counts['Andrew E. Slaughter'] > 0)
        self.assertEqual(n_no_blank, sum(list(counts.values())))

        counts = mooseutils.git_lines(__file__, blank=True)
        self.assertIn('Andrew E. Slaughter', counts)
        self.assertTrue(counts['Andrew E. Slaughter'] > 0)
        self.assertEqual(n_with_blank, sum(list(counts.values())))

    def testGitLocalPath(self):
        filename = os.path.abspath(__file__)
        local = mooseutils.git_localpath(filename)
        self.assertEqual(local, 'python/mooseutils/tests/test_gitutils.py')

    @mock.patch('subprocess.run')
    def testGitRepo(self, mock_out):
        mock_out.return_value.stdout = 'origin git@github.com:aeslaughter/moose.git (fetch)\n' \
                                       'origin git@github.com:aeslaughter/moose.git (push)\n' \
                                       'upstream git@github.com:idaholab/moose.git (fetch)' \
                                       'upstream git@github.com:idaholab/moose.git (push)'

        url = mooseutils.git_repo(os.path.dirname(__file__))
        self.assertEqual(url, 'https://github.com/idaholab/moose')

        url = mooseutils.git_repo(os.path.dirname(__file__), remotes=['origin'])
        self.assertEqual(url, 'https://github.com/aeslaughter/moose')

        with self.assertRaises(OSError) as e:
            mooseutils.git_repo('wrong')
        self.assertEqual(str(e.exception), "The supplied location must be a directory: wrong")

        with self.assertRaises(OSError) as e:
            mooseutils.git_repo(os.path.dirname(__file__), remotes=['wrong'])
        self.assertEqual(str(e.exception), "Unable to locate a remote with the name(s): wrong")

    def testGitIsConfig(self):
        with mock.patch('mooseutils.check_output', return_value='moosetest') as mock_check_output:
            self.assertTrue(mooseutils.git_is_config('user.name', 'moosetest', '/working/dir'))
        mock_check_output.assert_called_with(['git', 'config', '--get', 'user.name'], cwd='/working/dir')

    def testGitIsBranch(self):
        with mock.patch('mooseutils.check_output', side_effect=['true', 'main']) as mock_check_output:
            self.assertTrue(mooseutils.git_is_branch('main', '/working/dir'))
        mock_check_output.assert_called_with(['git', 'rev-parse', '--abbrev-ref', 'HEAD'], cwd='/working/dir')

        with mock.patch('mooseutils.check_output', side_effect=['false']) as mock_check_output:
            self.assertFalse(mooseutils.git_is_branch('main', '/working/dir'))
        mock_check_output.assert_called_once()

if __name__ == '__main__':
    unittest.main(verbosity=2, buffer=True)
