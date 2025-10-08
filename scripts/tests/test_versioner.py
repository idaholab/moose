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
from mock import patch, MagicMock
from io import StringIO
import tempfile
import shutil

MOOSE_DIR = mooseutils.git_root_dir()
sys.path.insert(0, os.path.join(MOOSE_DIR, 'scripts'))
from versioner import Versioner, Package, CondaPackage, MOOSE_DIR

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

class TestVerify(unittest.TestCase):
    def setUp(self):
        # Mock CLI args sent to verify_recipes
        self.args = MagicMock(verify="qwertyuiop", brief=False)
        # Remove color from tables for easy comparison
        self._colorText_patch = patch("versioner.colorText", side_effect=lambda s, c: s)
        self._colorText_mock = self._colorText_patch.start()
        # Don't actually tabulate, we'll just get the args used
        self._tabulate_patch = patch("tabulate.tabulate")
        self.tabulate_mock = self._tabulate_patch.start()
        # Mock stdout so we can more easily get print statements
        self._stdout_patcher = patch("sys.stdout", new=StringIO())
        self.stdout_mock: StringIO = self._stdout_patcher.start()
        # Temporary directory
        self._temp_dir = tempfile.mkdtemp()

    def tearDown(self):
        self._colorText_patch.stop()
        self._tabulate_patch.stop()
        self._stdout_patcher.stop()
        shutil.rmtree(self._temp_dir, ignore_errors=True)

    class MockGetPackages:
        def __init__(self, head: list[Package], base: list[Package]):
            self.head = {p.name: p for p in head}
            self.base = {p.name: p for p in base}

        def get_packages(self, ref: str) -> dict[str, Package]:
            return self.head if ref == "HEAD" else self.base

    @staticmethod
    def buildMockPackage(**kwargs) -> Package:
        name = kwargs.get("name", "mock")
        influential = kwargs.get("influential", {name: {}})
        conda = CondaPackage(
            name=name,
            version=kwargs.get("version", "1993.11.03"),
            build_number=kwargs.get("build_number", None),
            build_string=str(kwargs.get("build_number", "")) or None,
            install=kwargs.get("version", "1993.11.03"),
            conda_dir=None,
            meta={},
        )
        return Package(
            name=name,
            config=kwargs.get("config", {}),
            version=kwargs.get("version", "1993.11.03"),
            build_number=kwargs.get("build_number", None),
            full_version=kwargs.get("full_version", "1993.11.03"),
            hash=kwargs.get("hash", "asdfghjkl"),
            all_influential=kwargs.get("all_influential", influential[name]),
            influential=kwargs.get("influential", influential),
            dependencies=kwargs.get("dependencies", []),
            templates=kwargs.get("templates", {}),
            apptainer=kwargs.get("apptainer", None),
            conda=conda,
            is_app=kwargs.get("is_app", False),
        )

    @patch("versioner.Versioner.get_packages")
    def testNoChanges(self, mock_get_packages):
        head = [self.buildMockPackage()]
        base = [self.buildMockPackage()]
        mock_get_packages.side_effect = self.MockGetPackages(head, base).get_packages

        Versioner().verify_recipes(self.args)
        output = self.stdout_mock.getvalue()

        # All table headers are there
        self.assertIn("Versioner templates", output)
        self.assertIn("Versioner influential files", output)
        self.assertIn("Versioner versions", output)

        # Check summaries
        self.assertIn("Found 0 templates, 0 failed", output)
        self.assertIn("Found 0 influential files, 0 changed, 0 added, 0 removed", output)

        # Check version table
        data = self.tabulate_mock.call_args_list[0][0][0]
        headers = self.tabulate_mock.call_args_list[0][1]["headers"]
        self.assertListEqual(data, [['mock', 'OK', 'asdfghjkl', 'asdfghjkl', '1993.11.03', '1993.11.03']])
        self.assertListEqual(headers, ['package', 'status', 'hash', 'to hash', 'version', 'to version'])

        # Check success message
        self.assertIn("Verification succeeded.", output)

    @patch("versioner.Versioner.get_packages")
    def testSuccessfulChange(self, mock_get_packages):
        head = [self.buildMockPackage(version="2019.06.10", hash="qwertyuiop")]
        base = [self.buildMockPackage()]
        mock_get_packages.side_effect = self.MockGetPackages(head, base).get_packages

        Versioner().verify_recipes(self.args)
        output = self.stdout_mock.getvalue()

        data = self.tabulate_mock.call_args_list[0][0][0]
        headers = self.tabulate_mock.call_args_list[0][1]["headers"]
        self.assertListEqual(data, [['mock', 'CHANGE', 'asdfghjkl', 'qwertyuiop', '1993.11.03', '2019.06.10']])
        self.assertListEqual(headers, ['package', 'status', 'hash', 'to hash', 'version', 'to version'])

        self.assertIn("Changes were found in 1 packages.", output)
        self.assertIn("Verification succeeded.", output)

    @patch("versioner.Versioner.get_packages")
    def testNewPackage(self, mock_get_packages):
        head = [
            self.buildMockPackage(),
            self.buildMockPackage(name="sparkly"),
        ]
        base = [self.buildMockPackage()]
        mock_get_packages.side_effect = self.MockGetPackages(head, base).get_packages

        Versioner().verify_recipes(self.args)
        output = self.stdout_mock.getvalue()

        data = self.tabulate_mock.call_args_list[0][0][0]
        headers = self.tabulate_mock.call_args_list[0][1]["headers"]
        self.assertListEqual(headers, ['package', 'status', 'hash', 'to hash', 'version', 'to version'])
        self.assertListEqual(
            data,
            [
                ['mock', 'OK', 'asdfghjkl', 'asdfghjkl', '1993.11.03', '1993.11.03'],
                ['sparkly', 'NEW', 'none', 'asdfghjkl', 'none', '1993.11.03'],
            ]
        )

        self.assertIn("Changes were found in 1 packages.", output)
        self.assertIn("Verification succeeded.", output)

    def testBadChanges(self):

        def basicCheck(head, base) -> list[list[str]]:
            self.stdout_mock.truncate(0)
            with patch("versioner.Versioner.get_packages") as mock_get_packages:
                mock_get_packages.side_effect = self.MockGetPackages([head], [base]).get_packages
                with self.assertRaises(SystemExit):
                    Versioner().verify_recipes(self.args)
            output = self.stdout_mock.getvalue()
            headers = self.tabulate_mock.call_args_list[0][1]["headers"]
            self.assertListEqual(headers, ['package', 'status', 'hash', 'to hash', 'version', 'to version'])
            self.assertIn("Changes were found in 1 packages.", output)
            self.assertIn("Verification failed.", output)
            return self.tabulate_mock.call_args_list[-1][0][0]

        # Hash changed but no version bump
        data = basicCheck(self.buildMockPackage(hash="qwertyuiop"), self.buildMockPackage())
        self.assertListEqual(data, [['mock', 'NEED BUMP', 'asdfghjkl', 'qwertyuiop', '1993.11.03', '1993.11.03']])

        # Date decrease
        data = basicCheck(self.buildMockPackage(version="1991.05.21", hash="qwertyuiop"), self.buildMockPackage())
        self.assertListEqual(data, [['mock', 'DATE DECREASE', 'asdfghjkl', 'qwertyuiop', '1993.11.03', '1991.05.21']])

        # Future date
        data = basicCheck(self.buildMockPackage(version="3001.01.01", hash="qwertyuiop"), self.buildMockPackage())
        self.assertListEqual(data, [['mock', 'FUTURE DATE', 'asdfghjkl', 'qwertyuiop', '1993.11.03', '3001.01.01']])

        # Build non-zero
        data = basicCheck(self.buildMockPackage(version="2019.06.10", hash="qwertyuiop", build_number=1), self.buildMockPackage())
        self.assertListEqual(data, [['mock', 'BUILD NONZERO', 'asdfghjkl', 'qwertyuiop', '1993.11.03', '2019.06.10 build 1']])

    @patch("versioner.Versioner.git_file")
    @patch("versioner.Versioner.get_packages")
    def testTemplate(self, mock_get_packages, mock_git_file):
        # Don't search commit and just read the file
        def mockGitFile(file, commit, *args, **kwargs):
            with open(file) as fid:
                content = fid.read()
            return content
        mock_git_file.side_effect = mockGitFile

        # File paths of template-file pair
        file = os.path.join(self._temp_dir, "mock.yaml")
        template_file = os.path.join(self._temp_dir, "mock.yaml.template")
        rel_template_file = os.path.relpath(template_file, MOOSE_DIR)
        templates = {rel_template_file: file}

        # Create packages with templates
        head = [self.buildMockPackage(templates=templates)]
        base = [self.buildMockPackage(templates=templates)]
        mock_get_packages.side_effect = self.MockGetPackages(head, base).get_packages

        # Good template
        with open(template_file, "w") as fid:
            fid.write("__VERSIONER_MOCK_VERSION__")
        with open(file, "w") as fid:
            fid.write("1993.11.03")
        Versioner().verify_recipes(self.args)
        output = self.stdout_mock.getvalue()
        headers = self.tabulate_mock.call_args_list[-3][1]["headers"]
        data = self.tabulate_mock.call_args_list[-3][0][0]
        self.assertListEqual(headers, ['package', 'status', 'file', 'from'])
        self.assertListEqual(data, [['mock', 'OK', file, rel_template_file]])
        self.assertIn("Verification succeeded.", output)

        # Bad template
        self.stdout_mock.truncate(0)
        with open(file, "w") as fid:
            fid.write("2019.06.10")
        with self.assertRaises(SystemExit):
            Versioner().verify_recipes(self.args)
        output = self.stdout_mock.getvalue()
        headers = self.tabulate_mock.call_args_list[-3][1]["headers"]
        data = self.tabulate_mock.call_args_list[-3][0][0]
        self.assertListEqual(headers, ['package', 'status', 'file', 'from'])
        self.assertListEqual(data, [['mock', 'BEHIND', file, rel_template_file]])
        self.assertIn("Verification failed.", output)

        # Unused template variable
        self.stdout_mock.truncate(0)
        with open(template_file, "w") as fid:
            fid.write("__VERSIONER_MOCK_FOOBAR__")
        with self.assertRaises(SystemExit):
            Versioner().verify_recipes(self.args)
        output = self.stdout_mock.getvalue()
        self.assertIn(f"Unused template variable __VERSIONER_MOCK_FOOBAR__ still exists in {rel_template_file}", output)


    @patch("versioner.Versioner.get_packages")
    def testInfluential(self, mock_get_packages):
        head = [
            self.buildMockPackage(influential={"mock": {
                "not_changed": "qwerty",
                "changed": "qwerty",
                "added": "qwerty",
            }}),
            self.buildMockPackage(name="sparkly", influential={"sparkly": {
                "influence": "qwerty",
            }}),
        ]
        base = [self.buildMockPackage(influential={"mock": {
            "not_changed": "qwerty",
            "changed": "asdfgh",
            "removed": "qwerty",
        }})]
        mock_get_packages.side_effect = self.MockGetPackages(head, base).get_packages

        Versioner().verify_recipes(self.args)
        headers = self.tabulate_mock.call_args_list[0][1]["headers"]
        self.assertListEqual(headers, ['package', 'status', 'file'])
        data = self.tabulate_mock.call_args_list[0][0][0]
        self.assertIn(['mock', 'CHANGE', 'changed'], data)
        self.assertIn(['mock', 'NEW', 'added'], data)
        self.assertIn(['mock', 'REMOVED', 'removed'], data)
        self.assertIn(['sparkly', 'NEW', 'influence'], data)

        output = self.stdout_mock.getvalue()
        self.assertIn("Verification succeeded.", output)


if __name__ == '__main__':
    unittest.main(verbosity=2, buffer=True)
