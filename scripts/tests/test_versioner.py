#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://mooseframework.inl.gov
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import os
import sys
import unittest
import yaml
import subprocess # for assertRaises
import mooseutils
from mock import patch

MOOSE_DIR = mooseutils.git_root_dir()
sys.path.insert(0, os.path.join(MOOSE_DIR, 'scripts'))
from versioner import Versioner

with open(os.path.join(MOOSE_DIR, 'scripts', 'tests', 'versioner_hashes.yaml'), 'r') as stream:
    OLD_HASHES = yaml.safe_load(stream)

HEAD = Versioner.git_rev_parse('HEAD')

class Test(unittest.TestCase):
    def testOldHashes(self):
        for commit, libraries in OLD_HASHES.items():
            packages = Versioner().get_packages(commit)
            for name, values in libraries.items():
                package = packages[name]
                for key in ['hash', 'full_version']:
                    value = values.get(key)
                    if value:
                        self.assertEqual(value,
                                         getattr(package, key),
                                         f'{name}.{key} at {commit}')

    def testBadCommit(self):
        with self.assertRaises(Exception) as e:
            Versioner().get_packages('foobar')
        self.assertIn('Reference foobar is not valid', str(e.exception))

    def testCLI(self):
        versioner = Versioner()
        packages = versioner.get_packages('HEAD')

        for hash in [None, 'HEAD']:
            for name in ['tools', 'moose-dev']:
                def run(library, args=[]):
                    full_args = [library]
                    if hash is not None:
                        full_args += [hash]
                    full_args += args

                    return mooseutils.check_output(['./versioner.py'] + full_args,
                                                   cwd=os.path.join(mooseutils.git_root_dir(), 'scripts')).rstrip()
                package = packages[name]

                # default: just the hash
                cli_version = run(name)
                self.assertEqual(cli_version, package.full_version)

                # --json
                cli_json = run(name, ['--json'])
                self.assertIn(f'"name": "{name}"', cli_json)
                self.assertIn(f'"full_version": "{package.full_version}"', cli_json)

                # --yaml
                cli_yaml = run(name, ['--yaml'])
                self.assertIn(f'name: {name}', cli_yaml)
                self.assertIn(f'full_version: {package.full_version}', cli_yaml)

    def testGitHash(self):
        self.assertEqual(Versioner.git_hash('LICENSE', '6724fe26513b2a4f458f5fe8dbd253c2059bda59'), '4362b49151d7b34ef83b3067a8f9c9f877d72a0e')
        self.assertEqual(Versioner.git_hash('README.md', '21bace124ee2bfc46350b1f3540accab5d1ab0bb'), 'd801a34a243d319e79991b299aa3eb8e8fe937c0')

        with self.assertRaises(Exception) as e:
            Versioner.git_hash('foobar', 'HEAD')
        self.assertIn('Failed to obtain git hash', str(e.exception))

    def testGitIsCommit(self):
        self.assertTrue(Versioner.git_is_commit(HEAD))
        self.assertFalse(Versioner.git_is_commit('foo'))

    def testGitAncestor(self):
        ancestor = '4c082c170ff5295bcc94721c3da4131ceacae727'
        dependent = 'dd8042e2116537945caf305320eeeb526c15ff88'
        self.assertTrue(Versioner.git_ancestor(ancestor, dependent))
        self.assertFalse(Versioner.git_ancestor(dependent, ancestor))

        with self.assertRaises(subprocess.CalledProcessError):
            Versioner.git_ancestor('foo', 'bar')

    @patch.object(Versioner, 'git_is_commit')
    def testVersionerYamlPathMissingCommit(self, p):
        """
        Tests that versioner_yaml_path will return the new
        versioner.yaml path if the changed commit does not exist
        (like with a shallow clone)
        """
        p.return_value = False
        self.assertIn('versioner.yaml', Versioner.versioner_yaml_path(HEAD))
        p.assert_called_once()

    @patch.object(Versioner, 'git_is_commit')
    def testUsingOldInfluentialMissingCommit(self, p):
        """
        Tests that using_old_influential will return False if the
        changed commit does not exist (like with a shallow clone)
        """
        p.return_value = False
        self.assertFalse(Versioner.using_old_influential(HEAD))
        p.assert_called_once()

    @patch.object(Versioner, 'git_is_commit')
    def testUsingManagedVersionsMissingCommit(self, p):
        """
        Tests that using_managed_versions will return True if the
        changed commit does not exist (like with a shallow clone)
        """
        p.return_value = False
        self.assertTrue(Versioner.using_managed_versions(HEAD))
        p.assert_called_once()

    def testGitFile(self):
        self.assertNotIn('thm', Versioner.git_file('.coverage', 'bc75e4a07af609e3cfbc5c789aa69a8b3fc2c099'))
        self.assertIn('thm', Versioner.git_file('.coverage', '3ce960385af649e3a0737ed445410bcb998b2267'))
        self.assertEqual(None, Versioner.git_file('foo', 'HEAD', allow_missing=True))

        with self.assertRaises(Exception) as e:
            Versioner.git_file('foo', 'HEAD')
        self.assertIn('Failed to load', str(e.exception))

        out_of_repo_path = os.path.join(MOOSE_DIR, '..', 'out_of_repo')
        with self.assertRaises(Exception) as e:
            Versioner.git_file(out_of_repo_path, 'HEAD')
        self.assertEqual(f'Supplied path {out_of_repo_path} is not in {MOOSE_DIR}', str(e.exception))

    def testGetApp(self):
        app_info = Versioner.get_app_info()
        self.assertEqual('moose', app_info.name)
        self.assertEqual(MOOSE_DIR, app_info.git_root)
        self.assertEqual(Versioner.git_rev_parse('HEAD')[0:7], app_info.hash)

    def testMatchDate(self):
        self.assertEqual(Versioner.match_date('2025.05.05'), (2025, 5, 5))
        self.assertEqual(Versioner.match_date('xxx2025.04.04xxx'), (2025, 4, 4))
        self.assertEqual(Versioner.match_date('20.01.01'), None)

if __name__ == '__main__':
    unittest.main(verbosity=2, buffer=True)
