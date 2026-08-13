#!/usr/bin/env python3
# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details

import os
import tempfile
import unittest

import mooseutils
from moosesqa import (
    check_unit_test_sqa,
    discover_google_tests,
    discover_unit_test_directories,
)


class TestDiscoverGoogleTests(unittest.TestCase):
    def testDeclarations(self):
        source = r"""
TEST(SingleSuite, singleCase) {}
TEST_F(
    FixtureSuite,
    multilineCase)
{}
TEST_P(ParameterizedSuite, declarationCase) {}
TYPED_TEST(TypedSuite, typedCase) {}
TYPED_TEST_P(TypedPatternSuite, typedPatternCase) {}
INSTANTIATE_TEST_SUITE_P(Prefix, ParameterizedSuite, Values(1, 2));
REGISTER_TYPED_TEST_SUITE_P(TypedPatternSuite, typedPatternCase);

// TEST(CommentSuite, commentCase) {}
const char * normal = "TEST(StringSuite, stringCase)";
const char * raw = R"raw(TEST(RawStringSuite, rawStringCase))raw";
"""
        tests = discover_google_tests("Example.C", content=source)

        self.assertEqual(
            [test.identifier for test in tests],
            [
                "SingleSuite.singleCase",
                "FixtureSuite.multilineCase",
                "ParameterizedSuite.declarationCase",
                "TypedSuite.typedCase",
                "TypedPatternSuite.typedPatternCase",
            ],
        )
        self.assertEqual([test.line for test in tests], [2, 3, 7, 8, 9])

    def testWrapperMacro(self):
        source = r"""
#define MAKE_OPERATION_TEST(name) \
  TEST(GeneratedSuite, name)      \
  {                               \
  }

MAKE_OPERATION_TEST(first);
MAKE_OPERATION_TEST(second);
"""
        tests = discover_google_tests("Wrapper.C", content=source)

        self.assertEqual(
            [test.identifier for test in tests],
            ["GeneratedSuite.first", "GeneratedSuite.second"],
        )
        self.assertEqual([test.line for test in tests], [7, 8])

    def testDuplicateDeclarations(self):
        source = """
TEST(DuplicateSuite, duplicateCase) {}
TEST(DuplicateSuite, duplicateCase) {}
"""
        tests = discover_google_tests("Duplicate.C", content=source)
        self.assertEqual(
            [test.identifier for test in tests],
            ["DuplicateSuite.duplicateCase", "DuplicateSuite.duplicateCase"],
        )


class TestCheckUnitTestSQA(unittest.TestCase):
    def setUp(self):
        self._temporary_directory = tempfile.TemporaryDirectory()
        self.root = self._temporary_directory.name
        self.source = "unit/src/Example.C"
        self.metadata = "unit/src/Example.unit_tests"
        self.manifest = "legacy.yml"
        self._write(self.source, "TEST(ExampleSuite, exampleCase) {}\n")
        self._write(self.manifest, "{}\n")

    def tearDown(self):
        self._temporary_directory.cleanup()

    def _write(self, relative, content):
        filename = os.path.join(self.root, relative)
        os.makedirs(os.path.dirname(filename), exist_ok=True)
        with open(filename, "w", encoding="utf-8") as stream:
            stream.write(content)

    def _metadata_content(self, identifiers, include_required=True):
        blocks = []
        for i, identifier in enumerate(identifiers):
            required = (
                """
    requirement = 'The system shall provide example behavior.'
    design = 'Example.md'
    issues = '#33325'"""
                if include_required
                else ""
            )
            blocks.append("""  [example_{}]
    type = GoogleTest
    unit_test = {}{}
  []""".format(i, identifier, required))
        return "[Tests]\n{}\n[]\n".format("\n".join(blocks))

    def _check(self, extra_files=None):
        tracked = [self.source, self.manifest]
        if os.path.isfile(os.path.join(self.root, self.metadata)):
            tracked.append(self.metadata)
        tracked.extend(extra_files or [])
        return check_unit_test_sqa(
            self.root,
            legacy_manifest=self.manifest,
            tracked_files=tracked,
        )

    def testDocumented(self):
        self._write(
            self.metadata,
            self._metadata_content(["ExampleSuite.exampleCase"]),
        )
        self.assertEqual(self._check(), [])

    def testLegacy(self):
        self._write(
            self.manifest,
            "unit:\n  - ExampleSuite.exampleCase\n",
        )
        self.assertEqual(self._check(), [])

    def testMissing(self):
        diagnostics = self._check()
        self.assertEqual(len(diagnostics), 1)
        self.assertIn("has no SQA metadata", diagnostics[0].message)
        self.assertEqual(diagnostics[0].filename, self.source)
        self.assertEqual(diagnostics[0].line, 1)

    def testStaleMetadata(self):
        self._write(
            self.metadata,
            self._metadata_content(["ExampleSuite.oldCase"]),
        )
        messages = [item.message for item in self._check()]
        self.assertTrue(any("stale SQA entry" in message for message in messages))
        self.assertTrue(any("has no SQA metadata" in message for message in messages))

    def testStaleLegacy(self):
        self._write(
            self.manifest,
            "unit:\n  - ExampleSuite.oldCase\n",
        )
        messages = [item.message for item in self._check()]
        self.assertTrue(any("stale legacy entry" in message for message in messages))
        self.assertTrue(any("has no SQA metadata" in message for message in messages))

    def testDuplicateMetadata(self):
        self._write(
            self.metadata,
            self._metadata_content(
                ["ExampleSuite.exampleCase", "ExampleSuite.exampleCase"]
            ),
        )
        messages = [item.message for item in self._check()]
        self.assertEqual(
            len(
                [message for message in messages if "duplicate SQA mapping" in message]
            ),
            2,
        )

    def testDocumentedAndLegacy(self):
        self._write(
            self.metadata,
            self._metadata_content(["ExampleSuite.exampleCase"]),
        )
        self._write(
            self.manifest,
            "unit:\n  - ExampleSuite.exampleCase\n",
        )
        messages = [item.message for item in self._check()]
        self.assertTrue(
            any(
                "both SQA metadata and the legacy manifest" in message
                for message in messages
            )
        )

    def testMissingRequiredMetadata(self):
        self._write(
            self.metadata,
            self._metadata_content(
                ["ExampleSuite.exampleCase"], include_required=False
            ),
        )
        messages = [item.message for item in self._check()]
        for parameter in ("requirement", "design", "issues"):
            self.assertTrue(
                any(
                    "has no '{}' metadata".format(parameter) in message
                    for message in messages
                )
            )

    def testGroupedMetadata(self):
        self._write(
            self.source,
            """
TEST(ExampleSuite, firstCase) {}
TEST(ExampleSuite, secondCase) {}
""",
        )
        self._write(
            self.metadata,
            """[Tests]
  [example_group]
    requirement = 'The system shall provide grouped example behavior.'
    design = 'Example.md'
    issues = '#33325'
    [first]
      type = GoogleTest
      unit_test = ExampleSuite.firstCase
      detail = 'with the first input; and'
    []
    [second]
      type = GoogleTest
      unit_test = ExampleSuite.secondCase
      detail = 'with the second input.'
    []
  []
[]
""",
        )
        self.assertEqual(self._check(), [])

    def testGroupedMetadataRequiresDetail(self):
        self._write(
            self.metadata,
            """[Tests]
  [example_group]
    requirement = 'The system shall provide grouped example behavior.'
    design = 'Example.md'
    issues = '#33325'
    [example]
      type = GoogleTest
      unit_test = ExampleSuite.exampleCase
    []
  []
[]
""",
        )
        diagnostics = self._check()
        self.assertTrue(any("has no 'detail'" in item.message for item in diagnostics))

    def testMetadataMustMatchSource(self):
        unmatched = "unit/src/Other.unit_tests"
        self._write(
            unmatched,
            self._metadata_content(["ExampleSuite.exampleCase"]),
        )
        diagnostics = self._check(extra_files=[unmatched])
        self.assertTrue(
            any(
                "must have exactly one matching .C or .K source" in item.message
                for item in diagnostics
            )
        )

    def testDuplicateDeclarations(self):
        self._write(
            self.source,
            """
TEST(ExampleSuite, exampleCase) {}
TEST(ExampleSuite, exampleCase) {}
""",
        )
        self._write(
            self.manifest,
            "unit:\n  - ExampleSuite.exampleCase\n",
        )
        diagnostics = self._check()
        self.assertEqual(
            len(
                [
                    item
                    for item in diagnostics
                    if "duplicate logical GoogleTest identifier" in item.message
                ]
            ),
            2,
        )

    def testNoTestsNoManifest(self):
        # A directory with nothing to check and no manifest is not an error.
        diagnostics = check_unit_test_sqa(
            self.root,
            legacy_manifest=os.path.join(self.root, "missing.yml"),
            tracked_files=[],
        )
        self.assertEqual(diagnostics, [])

    def testMissingManifestNoGrandfathering(self):
        # A missing manifest grants no legacy exemptions; every test needs metadata.
        diagnostics = check_unit_test_sqa(
            self.root,
            legacy_manifest=os.path.join(self.root, "missing.yml"),
            tracked_files=[self.source],
        )
        self.assertEqual(len(diagnostics), 1)
        self.assertIn("has no SQA metadata", diagnostics[0].message)
        self.assertEqual(diagnostics[0].filename, self.source)

    def testDefaultManifestPath(self):
        default_manifest = "doc/legacy_unit_tests.yml"
        self._write(default_manifest, "unit:\n  - ExampleSuite.exampleCase\n")
        diagnostics = check_unit_test_sqa(
            self.root, tracked_files=[self.source, default_manifest]
        )
        self.assertEqual(diagnostics, [])


class TestDiscoverUnitTestDirectories(unittest.TestCase):
    def setUp(self):
        self._temporary_directory = tempfile.TemporaryDirectory()
        self.root = self._temporary_directory.name

    def tearDown(self):
        self._temporary_directory.cleanup()

    def testRootAndNestedModule(self):
        # A 'framework/' directory on disk marks this as the MOOSE repository itself, so
        # the root-level ("") module's legacy manifest lives under framework/doc.
        os.makedirs(os.path.join(self.root, "framework"))
        tracked = [
            "unit/src/Example.C",
            "modules/contact/unit/src/Example.C",
        ]
        results = discover_unit_test_directories(self.root, tracked_files=tracked)
        self.assertEqual(
            results,
            [
                (
                    "",
                    os.path.join(
                        self.root, "framework", "doc", "legacy_unit_tests.yml"
                    ),
                ),
                (
                    "modules/contact",
                    os.path.join(
                        self.root, "modules", "contact", "doc", "legacy_unit_tests.yml"
                    ),
                ),
            ],
        )

    def testDownstreamAppRoot(self):
        # No 'framework/' directory on disk: this is a downstream app, whose root-level
        # ("") module's legacy manifest is a sibling 'doc/', like any other module.
        tracked = ["unit/src/Example.C"]
        results = discover_unit_test_directories(self.root, tracked_files=tracked)
        self.assertEqual(
            results,
            [("", os.path.join(self.root, "doc", "legacy_unit_tests.yml"))],
        )


class TestRepositoryUnitTestSQA(unittest.TestCase):
    def testRepository(self):
        root = os.path.abspath(
            os.path.join(os.path.dirname(__file__), "..", "..", "..")
        )
        tracked = list(mooseutils.git_ls_files(root))
        new_metadata = os.path.join(root, "unit", "src", "MooseUtilsTest.unit_tests")
        if new_metadata not in tracked:
            tracked.append(new_metadata)

        for module_root, legacy_manifest in discover_unit_test_directories(
            root, tracked_files=tracked
        ):
            # Scope to this module's own files -- 'unit/src/' is matched at any depth,
            # so an unfiltered list would let the root ("") case sweep in every other
            # module's nested unit/src directory too.
            prefix = module_root + "/" if module_root else "unit/"
            module_tracked = [
                filename
                for filename in tracked
                if os.path.relpath(filename, root).replace(os.sep, "/").startswith(
                    prefix
                )
            ]
            unit_root = os.path.join(root, module_root) if module_root else root
            diagnostics = check_unit_test_sqa(
                unit_root, legacy_manifest=legacy_manifest, tracked_files=module_tracked
            )
            self.assertEqual(
                diagnostics,
                [],
                "{}:\n".format(module_root or "framework")
                + "\n".join(
                    "{}:{}: {}".format(item.filename, item.line, item.message)
                    for item in diagnostics
                ),
            )


if __name__ == "__main__":
    unittest.main(verbosity=2)
