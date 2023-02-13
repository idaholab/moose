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
import sys
import unittest
import yaml
import subprocess # for assertRaises
import mooseutils
import json
import platform
from io import StringIO
from unittest.mock import patch

MOOSE_DIR = mooseutils.git_root_dir()
sys.path.insert(0, os.path.join(MOOSE_DIR, 'scripts'))
from versioner import Versioner

with open(os.path.join(MOOSE_DIR, 'scripts', 'tests', 'versioner_hashes.yaml'), 'r') as stream:
    OLD_HASHES = yaml.safe_load(stream)

class Test(unittest.TestCase):
    def testOldHashes(self):
        versioner = Versioner()
        for hash, packages in OLD_HASHES.items():
            meta = versioner.meta(hash)
            for package, package_hash in packages.items():
                self.assertEqual(package_hash, meta[package]['hash'])

    def testBadCommit(self):
        with self.assertRaises(Exception) as e:
            Versioner().meta('foobar')
        self.assertIn('foobar is not a commit', str(e.exception))

    def testCLI(self):
        versioner = Versioner()
        meta = versioner.meta()

        for hash in [None, 'HEAD']:
            for package in versioner.entities:
                if package == 'app':
                    continue

                def run(library, args=[]):
                    full_args = [library]
                    if hash is not None:
                        full_args += [hash]
                    full_args += args
                    return mooseutils.check_output(['./versioner.py'] + full_args,
                                                cwd=os.path.join(mooseutils.git_root_dir(), 'scripts')).rstrip()
                package_meta = meta[package]

                # default: just the hash
                cli_hash = run(package)
                self.assertEqual(cli_hash, package_meta['hash'])

                # --json
                cli_json = run(package, ['--json'])
                self.assertEqual(cli_json, json.dumps(package_meta))

        # Can't pass multiple arguments
        with self.assertRaises(subprocess.CalledProcessError) as cm:
            mooseutils.check_output(['./versioner.py', '--json', '--yaml'],
                                    cwd=os.path.join(MOOSE_DIR, 'scripts'))

    def testIsGitObject(self):
        self.assertTrue(Versioner.is_git_object('HEAD'))
        self.assertTrue(Versioner.is_git_object('21bace124ee2bfc46350b1f3540accab5d1ab0bb'))
        self.assertFalse(Versioner.is_git_object('foobar'))

    def testGitHash(self):
        self.assertEqual(Versioner.git_hash('LICENSE', '6724fe26513b2a4f458f5fe8dbd253c2059bda59'), '4362b49151d7b34ef83b3067a8f9c9f877d72a0e')
        self.assertEqual(Versioner.git_hash('README.md', '21bace124ee2bfc46350b1f3540accab5d1ab0bb'), 'd801a34a243d319e79991b299aa3eb8e8fe937c0')

        with self.assertRaises(Exception) as e:
            Versioner.git_hash('foobar', 'HEAD')
        self.assertIn('Failed to obtain git hash', str(e.exception))

    def testGitAncestor(self):
        ancestor = '4c082c170ff5295bcc94721c3da4131ceacae727'
        dependent = 'dd8042e2116537945caf305320eeeb526c15ff88'
        self.assertTrue(Versioner.git_ancestor(ancestor, dependent))
        self.assertFalse(Versioner.git_ancestor(dependent, ancestor))

        with self.assertRaises(subprocess.CalledProcessError):
            Versioner.git_ancestor('foo', 'bar')

    def testGitFile(self):
        self.assertNotIn('thm', Versioner.git_file('.coverage', 'bc75e4a07af609e3cfbc5c789aa69a8b3fc2c099'))
        self.assertIn('thm', Versioner.git_file('.coverage', '3ce960385af649e3a0737ed445410bcb998b2267'))
        self.assertEqual(None, Versioner.git_file('foo', 'HEAD', allow_missing=True))

        with self.assertRaises(Exception) as e:
            Versioner.git_file('foo', 'HEAD')
        self.assertIn('Failed to load', str(e.exception))

    def testGetApp(self):
        app_name, git_root, git_hash = Versioner.get_app()
        self.assertEqual('moose-combined', app_name)
        self.assertEqual(MOOSE_DIR, git_root)

        # still need to test _in_ an app

    def testApptainerMeta(self):
        package_hash = 'abc1234'
        for package in ['libmesh', 'Some_app']:
            is_app = package != 'libmesh'
            def_package = 'app' if is_app else package
            name_prefix = '' if is_app else 'moose-'
            meta = Versioner.apptainer_meta(package, package_hash, is_app)
            if is_app:
                package = package.lower()
            self.assertEqual(meta['name'], f'{name_prefix}{package}-{platform.machine()}')
            self.assertEqual(meta['name_base'], f'{name_prefix}{package}')
            self.assertEqual(meta['name_suffix'], platform.machine())
            self.assertEqual(meta['tag'], package_hash)
            self.assertEqual(meta['uri'], f'{name_prefix}{package}-{platform.machine()}:{package_hash}')
            self.assertEqual(meta['def'], os.path.realpath(os.path.join(MOOSE_DIR, f'apptainer/{def_package}.def')))

if __name__ == '__main__':
    unittest.main(verbosity=2, buffer=True)
