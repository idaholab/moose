#!/usr/bin/env python3
# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html
import os
import re
import unittest
import mock
import types
import MooseDocs
from MooseDocs.commands import init


class TestInit(unittest.TestCase):
    def setUp(self):
        self._init_write = dict()
        self._yaml_write = dict()

        # Change to the test/doc directory
        self._working_dir = os.getcwd()
        moose_test_doc_dir = os.path.abspath(
            os.path.join(
                os.path.dirname(__file__), "..", "..", "..", "..", "test", "doc"
            )
        )
        os.chdir(moose_test_doc_dir)

    def tearDown(self):
        # Restore the working directory
        os.chdir(self._working_dir)

        self._init_write = None
        self._yanl_write = None

    def getCommandLineArguments(self, **kwargs):

        kwargs.setdefault("config", os.path.join(os.getcwd(), "config.yml"))
        kwargs.setdefault("command", "init")
        kwargs.setdefault("init_command", "sqa")
        kwargs.setdefault("app", None)
        kwargs.setdefault("module", None)

        return types.SimpleNamespace(**kwargs)

    def mockInitWrite(self, filename, content):
        self._init_write[filename.replace(MooseDocs.ROOT_DIR, "")] = content

    def mockYamlWrite(self, filename, content):
        self._yaml_write[filename.replace(MooseDocs.ROOT_DIR, "")] = content

    @mock.patch("mooseutils.git_repo")
    @mock.patch("mooseutils.IncludeYamlFile")
    @mock.patch("mooseutils.yaml_write")
    @mock.patch("MooseDocs.commands.init._write_file")
    def testSQA(self, mock_write_file, mock_yaml_write, proxy, mock_git_repo):
        mock_yaml_write.side_effect = self.mockYamlWrite
        mock_write_file.side_effect = self.mockInitWrite
        mock_git_repo.return_value = "test"

        opt = self.getCommandLineArguments(app="MooseTestApp", category="moose_test")
        status = init.main(opt)

        self.assertEqual(len(self._init_write.keys()), 12)

        self.assertIn("/test/doc/sqa_moose_test.yml", self._init_write.keys())
        self.assertIn(
            "/test/doc/content/sqa/moose_test_sdd.md", self._init_write.keys()
        )
        self.assertIn(
            "/test/doc/content/sqa/moose_test_srs.md", self._init_write.keys()
        )
        self.assertIn(
            "/test/doc/content/sqa/moose_test_stp.md", self._init_write.keys()
        )
        self.assertIn("/test/doc/content/sqa/index.md", self._init_write.keys())
        self.assertIn(
            "/test/doc/content/sqa/moose_test_rtm.md", self._init_write.keys()
        )
        self.assertIn(
            "/test/doc/content/sqa/moose_test_vvr.md", self._init_write.keys()
        )
        self.assertIn(
            "/test/doc/content/sqa/moose_test_far.md", self._init_write.keys()
        )
        self.assertIn(
            "/test/doc/content/sqa/moose_test_scs.md", self._init_write.keys()
        )
        self.assertIn(
            "/test/doc/content/sqa/moose_test_cci.md", self._init_write.keys()
        )
        self.assertIn(
            "/test/doc/content/sqa/moose_test_sll.md", self._init_write.keys()
        )
        self.assertIn("/test/doc/sqa_reports.yml", self._init_write.keys())

        self.assertEqual(len(self._yaml_write.keys()), 1)
        self.assertIn("/test/doc/config.yml", self._yaml_write.keys())

    def mockReadFile(self, filename):
        if filename.endswith("config.yml"):
            return self._fixture_config_yml
        return self._fixture_sqa_reports_yml

    @mock.patch("mooseutils.git_repo")
    @mock.patch("mooseutils.IncludeYamlFile")
    @mock.patch("mooseutils.yaml_write")
    @mock.patch("MooseDocs.commands.init._read_file")
    @mock.patch("MooseDocs.commands.init._write_file")
    def testSQAModule(
        self, mock_write_file, mock_read_file, mock_yaml_write, proxy, mock_git_repo
    ):
        mock_yaml_write.side_effect = self.mockYamlWrite
        mock_write_file.side_effect = self.mockInitWrite
        mock_read_file.side_effect = self.mockReadFile
        mock_git_repo.return_value = "test"

        self._fixture_config_yml = (
            "Extensions:\n"
            "    MooseDocs.extensions.sqa:\n"
            "        categories:\n"
            "            framework: !include framework/doc/sqa_framework.yml\n"
            "            alpha_mod: !include modules/alpha_mod/doc/sqa_alpha_mod.yml\n"
            "\n"
            "        requirement-groups:\n"
        )
        self._fixture_sqa_reports_yml = (
            "Applications:\n"
            "    framework:\n"
            "        exe_directory: modules/combined\n"
            "\n"
            "    alpha_mod:\n"
            "        exe_directory: modules/combined\n"
            "\n"
            "Documents:\n"
            "    software_quality_plan: sqa/inl_records.md\n"
            "\n"
            "Requirements:\n"
            "    create_diff_report: true\n"
            "\n"
            "    alpha_mod:\n"
            "        directories:\n"
            "            - modules/alpha_mod\n"
        )

        opt = self.getCommandLineArguments(module="Moose Test", category="moose_test")
        status = init.main(opt)

        self.assertIn("/test/doc/sqa_moose_test.yml", self._init_write.keys())
        self.assertIn("/test/doc/sqa_reports.yml", self._init_write.keys())
        self.assertIn(
            "/test/doc/content/sqa/moose_test_sdd.md", self._init_write.keys()
        )

        self.assertIn("/modules/doc/config.yml", self._init_write.keys())
        self.assertIn(
            "moose_test: !include modules/moose_test/doc/sqa_moose_test.yml",
            self._init_write["/modules/doc/config.yml"],
        )

        self.assertIn("/modules/doc/sqa_reports.yml", self._init_write.keys())
        self.assertIn(
            "    moose_test:\n        exe_directory: modules/combined",
            self._init_write["/modules/doc/sqa_reports.yml"],
        )
        self.assertIn(
            "    moose_test:\n        directories:\n            - modules/moose_test\n",
            self._init_write["/modules/doc/sqa_reports.yml"],
        )

    @mock.patch("MooseDocs.commands.init._read_file")
    @mock.patch("MooseDocs.commands.init._write_file")
    def testUpdateModuleSqaRegistrationInsertsAlphabetically(
        self, mock_write_file, mock_read_file
    ):
        mock_write_file.side_effect = self.mockInitWrite
        self._fixture_config_yml = (
            "Extensions:\n"
            "    MooseDocs.extensions.sqa:\n"
            "        categories:\n"
            "            framework: !include framework/doc/sqa_framework.yml\n"
            "            python: !include python/doc/sqa_python.yml\n"
            "            alpha_mod: !include modules/alpha_mod/doc/sqa_alpha_mod.yml\n"
            "            zeta_mod: !include modules/zeta_mod/doc/sqa_zeta_mod.yml\n"
            "\n"
            "        requirement-groups:\n"
            "            dgkernels: DGKernel Objects\n"
        )
        self._fixture_sqa_reports_yml = (
            "Applications:\n"
            "    framework:\n"
            "        exe_directory: modules/combined\n"
            "        app_types:\n"
            "            - MooseApp\n"
            "        content_directory: framework/doc/content\n"
            "        unregister:\n"
            "            - framework/doc/unregister.yml\n"
            "        remove:\n"
            "            - framework/doc/remove.yml\n"
            "\n"
            "    alpha_mod:\n"
            "        exe_directory: modules/combined\n"
            "        app_types:\n"
            "            - AlphaModApp\n"
            "        content_directory: modules/alpha_mod/doc/content\n"
            "        unregister:\n"
            "            - framework/doc/unregister.yml\n"
            "        remove:\n"
            "            - framework/doc/remove.yml\n"
            "\n"
            "    ZETA:\n"
            "        exe_directory: modules/combined\n"
            "        app_types:\n"
            "            - ZetaApp\n"
            "        content_directory: modules/zeta/doc/content\n"
            "        unregister:\n"
            "            - framework/doc/unregister.yml\n"
            "        remove:\n"
            "            - framework/doc/remove.yml\n"
            "\n"
            "Documents:\n"
            "    software_quality_plan: sqa/inl_records.md\n"
            "\n"
            "Requirements:\n"
            "    create_diff_report: true\n"
            "    moose_test:\n"
            "        directories:\n"
            "            - test/tests\n"
            "\n"
            "    alpha_mod:\n"
            "        directories:\n"
            "            - modules/alpha_mod\n"
            "\n"
            "    zeta:\n"
            "        directories:\n"
            "            - modules/zeta\n"
        )
        mock_read_file.side_effect = self.mockReadFile

        init.update_module_sqa_registration("Middle Mod", "middle_mod")

        self.assertEqual(
            self._init_write["/modules/doc/config.yml"],
            "Extensions:\n"
            "    MooseDocs.extensions.sqa:\n"
            "        categories:\n"
            "            framework: !include framework/doc/sqa_framework.yml\n"
            "            python: !include python/doc/sqa_python.yml\n"
            "            alpha_mod: !include modules/alpha_mod/doc/sqa_alpha_mod.yml\n"
            "            middle_mod: !include modules/middle_mod/doc/sqa_middle_mod.yml\n"
            "            zeta_mod: !include modules/zeta_mod/doc/sqa_zeta_mod.yml\n"
            "\n"
            "        requirement-groups:\n"
            "            dgkernels: DGKernel Objects\n",
        )

        self.assertEqual(
            self._init_write["/modules/doc/sqa_reports.yml"],
            "Applications:\n"
            "    framework:\n"
            "        exe_directory: modules/combined\n"
            "        app_types:\n"
            "            - MooseApp\n"
            "        content_directory: framework/doc/content\n"
            "        unregister:\n"
            "            - framework/doc/unregister.yml\n"
            "        remove:\n"
            "            - framework/doc/remove.yml\n"
            "\n"
            "    alpha_mod:\n"
            "        exe_directory: modules/combined\n"
            "        app_types:\n"
            "            - AlphaModApp\n"
            "        content_directory: modules/alpha_mod/doc/content\n"
            "        unregister:\n"
            "            - framework/doc/unregister.yml\n"
            "        remove:\n"
            "            - framework/doc/remove.yml\n"
            "\n"
            "    middle_mod:\n"
            "        exe_directory: modules/combined\n"
            "        app_types:\n"
            "            - MiddleModApp\n"
            "        content_directory: modules/middle_mod/doc/content\n"
            "        unregister:\n"
            "            - framework/doc/unregister.yml\n"
            "        remove:\n"
            "            - framework/doc/remove.yml\n"
            "\n"
            "    ZETA:\n"
            "        exe_directory: modules/combined\n"
            "        app_types:\n"
            "            - ZetaApp\n"
            "        content_directory: modules/zeta/doc/content\n"
            "        unregister:\n"
            "            - framework/doc/unregister.yml\n"
            "        remove:\n"
            "            - framework/doc/remove.yml\n"
            "\n"
            "Documents:\n"
            "    software_quality_plan: sqa/inl_records.md\n"
            "\n"
            "Requirements:\n"
            "    create_diff_report: true\n"
            "    moose_test:\n"
            "        directories:\n"
            "            - test/tests\n"
            "\n"
            "    alpha_mod:\n"
            "        directories:\n"
            "            - modules/alpha_mod\n"
            "\n"
            "    middle_mod:\n"
            "        directories:\n"
            "            - modules/middle_mod\n"
            "\n"
            "    zeta:\n"
            "        directories:\n"
            "            - modules/zeta\n",
        )

    @mock.patch("MooseDocs.commands.init._read_file")
    @mock.patch("MooseDocs.commands.init._write_file")
    def testUpdateModuleSqaRegistrationIdempotent(
        self, mock_write_file, mock_read_file
    ):
        mock_write_file.side_effect = self.mockInitWrite
        self._fixture_config_yml = (
            "Extensions:\n"
            "    MooseDocs.extensions.sqa:\n"
            "        categories:\n"
            "            framework: !include framework/doc/sqa_framework.yml\n"
            "            alpha_mod: !include modules/alpha_mod/doc/sqa_alpha_mod.yml\n"
            "\n"
            "        requirement-groups:\n"
        )
        self._fixture_sqa_reports_yml = (
            "Applications:\n"
            "    alpha_mod:\n"
            "        exe_directory: modules/combined\n"
            "\n"
            "Documents:\n"
            "    software_quality_plan: sqa/inl_records.md\n"
            "\n"
            "Requirements:\n"
            "    alpha_mod:\n"
            "        directories:\n"
            "            - modules/alpha_mod\n"
        )
        mock_read_file.side_effect = self.mockReadFile

        with self.assertLogs("MooseDocs.commands.init", level="WARNING") as cm:
            init.update_module_sqa_registration("Alpha Mod", "alpha_mod")

        self.assertEqual(len(self._init_write), 0)
        self.assertEqual(len(cm.output), 3)
        self.assertTrue(all("alpha_mod" in msg for msg in cm.output))


if __name__ == "__main__":
    unittest.main(verbosity=2)
