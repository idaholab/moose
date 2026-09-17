#!/usr/bin/env python3
# * This file is part of the MOOSE framework
# * https://mooseframework.inl.gov
# *
# * All rights reserved, see COPYRIGHT for full restrictions
# * https://github.com/idaholab/moose/blob/master/COPYRIGHT
# *
# * Licensed under LGPL 2.1, please see LICENSE for details
# * https://www.gnu.org/licenses/lgpl-2.1.html

import io
import json
import os
import subprocess
import sys
import tarfile
import tempfile
import unittest
from contextlib import redirect_stdout
from unittest import mock

sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), "..")))

import civet_pr_failures as cpf


class TestLatestStatuses(unittest.TestCase):
    def testNewestPerContextWins(self):
        # GitHub returns every status ever posted, newest first; CIVET posts
        # one when a job starts and another when it finishes.
        statuses = [
            {"context": "build", "state": "success"},
            {"context": "test", "state": "pending"},
            {"context": "build", "state": "pending"},
        ]
        self.assertEqual(
            cpf.latest_statuses(statuses),
            [
                {"context": "build", "state": "success"},
                {"context": "test", "state": "pending"},
            ],
        )

    def testMissingContextKeptOnce(self):
        statuses = [{"state": "success"}, {"state": "pending"}]
        self.assertEqual(cpf.latest_statuses(statuses), [{"state": "success"}])


class TestPendingAndRealFailures(unittest.TestCase):
    def testPendingJobs(self):
        statuses = [
            {"context": "a", "state": "pending"},
            {"context": "b", "state": "success"},
        ]
        self.assertEqual(cpf.pending_jobs(statuses), [statuses[0]])

    def testRealFailuresExcludesCascades(self):
        statuses = [
            {"context": "a", "state": "failure", "description": "boom"},
            {
                "context": "b",
                "state": "failure",
                "description": cpf.BLOCKED_DESCRIPTION,
            },
            {"context": "c", "state": "error", "description": "boom"},
            {"context": "d", "state": "success", "description": None},
        ]
        self.assertEqual(cpf.real_failures(statuses), [statuses[0], statuses[2]])


class TestDescribeTarget(unittest.TestCase):
    def testPullRequest(self):
        info = {
            "number": 123,
            "headRefName": "my-branch",
            "headRefOid": "abcdef0123456",
        }
        self.assertEqual(cpf.describe_target(info), "PR #123 my-branch @ abcdef0")

    def testBareCommit(self):
        info = {"number": None, "headRefName": None, "headRefOid": "abcdef0123456"}
        self.assertEqual(cpf.describe_target(info), "commit abcdef0")


class TestStripCiArgs(unittest.TestCase):
    def testDropsTimingAndLongestJobs(self):
        self.assertEqual(
            cpf.strip_ci_args(["-p", "4", "-t", "--longest-jobs=10", "--recover"]),
            ["-p", "4", "--recover"],
        )

    def testDropsValuedJobSlotArgs(self):
        self.assertEqual(
            cpf.strip_ci_args(["-j", "8", "--distributed-mesh"]), ["--distributed-mesh"]
        )

    def testKeepsModeAffectingArgs(self):
        args = [
            "--compute-device=cuda",
            "--max-memory-per-slot=1024",
            "--min-parallel",
            "7",
        ]
        self.assertEqual(cpf.strip_ci_args(args), args)


class TestExtractContainerAndInvocation(unittest.TestCase):
    def testExtractContainerTakesLastMatch(self):
        text = (
            "Executing step in docker://old:tag\nExecuting step in docker://new:tag\n"
        )
        self.assertEqual(cpf.extract_container(text), "docker://new:tag")

    def testExtractContainerNoneWhenAbsent(self):
        self.assertIsNone(cpf.extract_container("nothing to see here"))

    def testExtractInvocationTakesFirstMatch(self):
        text = (
            "[00:00:01] test: ./run_tests -p 4 --distributed-mesh\n"
            "[00:05:00] test: ./run_tests --failed-tests\n"
        )
        directory, args = cpf.extract_invocation(text)
        self.assertEqual(directory, "test")
        self.assertEqual(args, ["-p", "4", "--distributed-mesh"])

    def testExtractInvocationNoneWhenAbsent(self):
        self.assertIsNone(cpf.extract_invocation("no invocation logged here"))


class TestReproduceCommandAndModeIdentity(unittest.TestCase):
    def testNoInvocationFallsBackToBareRunTests(self):
        cmd = cpf.reproduce_command(None, "controls/web_server_control.connect_port")
        self.assertEqual(
            cmd, "./run_tests --re '^controls/web_server_control\\.connect_port$'"
        )

    def testInvocationAddsDirectoryAndArgs(self):
        invocation = ("test", ["-p", "4"])
        cmd = cpf.reproduce_command(invocation, "some.test")
        self.assertEqual(cmd, "cd test && ./run_tests -p 4 --re '^some\\.test$'")

    def testInvocationInCurrentDirectorySkipsCd(self):
        invocation = (".", ["-p", "4"])
        cmd = cpf.reproduce_command(invocation, "some.test")
        self.assertFalse(cmd.startswith("cd"))

    def testModeIdentityDropsMemoryLimitButKeepsRest(self):
        invocation = ("test", ["-p", "4", "--max-memory-per-slot=1024"])
        self.assertEqual(cpf.mode_identity(invocation), "-p 4")

    def testModeIdentityNoneInvocation(self):
        self.assertEqual(cpf.mode_identity(None), "")

    def testTwoModesDifferingOnlyInMemoryAreIdentical(self):
        a = ("test", ["-p", "4", "--max-memory-per-slot=1024"])
        b = ("test", ["-p", "4", "--max-memory-per-slot=2048"])
        self.assertEqual(cpf.mode_identity(a), cpf.mode_identity(b))


class TestRetryNote(unittest.TestCase):
    def testPassedOnRetry(self):
        self.assertEqual(cpf.retry_note(85), "passed on retry; intermittent")

    def testFailedAgainOnRetry(self):
        self.assertEqual(cpf.retry_note(1), "failed again on retry")

    def testNoRetryAttempted(self):
        self.assertEqual(cpf.retry_note(128), "no retry attempted")
        self.assertEqual(cpf.retry_note(0), "no retry attempted")


class TestNormalizeError(unittest.TestCase):
    def testTmpDirAddressAndDigitsAreNormalized(self):
        message = "opening /tmp/abc123/file at 0xdeadBEEF, attempt 3"
        self.assertEqual(
            cpf.normalize_error(message),
            "opening /tmp/<dir>/file at <addr>, attempt N",
        )


class TestErrorSignatures(unittest.TestCase):
    def testErrorBlockSkipsBoilerplateAndLocation(self):
        lines = [
            cpf.ERROR_BLOCK_MARKER,
            "The following occurred in the object foo",
            "input.i:10.5:",
            "  Real error message here  ",
            "trailing noise",
        ]
        signatures = cpf.error_signatures(lines)
        self.assertEqual(signatures["Real error message here"], 1)

    def testStandaloneErrorLine(self):
        lines = [
            "ValueError: bad value 123",
            "AssertionError: oops",
            "Fatal error: oops",
        ]
        signatures = cpf.error_signatures(lines)
        self.assertEqual(signatures["ValueError: bad value N"], 1)
        self.assertEqual(signatures["AssertionError: oops"], 1)
        self.assertEqual(signatures["Fatal error: oops"], 1)

    def testRepeatedSignatureIsCounted(self):
        lines = ["ValueError: bad value 1", "ValueError: bad value 2"]
        signatures = cpf.error_signatures(lines)
        self.assertEqual(signatures["ValueError: bad value N"], 2)


class TestHintsFor(unittest.TestCase):
    def testMatchingKeyProducesHint(self):
        hints = cpf.hints_for(["ERROR ... KILLED: OVER MEMORY ..."])
        self.assertEqual(len(hints), 1)
        self.assertTrue(hints[0].startswith("KILLED: OVER MEMORY -> "))

    def testNoMatchProducesNoHints(self):
        self.assertEqual(cpf.hints_for(["nothing recognizable here"]), [])


class TestDedupeTestFailures(unittest.TestCase):
    def testShorterAnnotationOfSameTestWins(self):
        lines = [
            "ERROR some.test FAILED (TIMEOUT, retry pending)",
            "ERROR some.test FAILED (TIMEOUT)",
        ]
        self.assertEqual(
            cpf.dedupe_test_failures(lines), ["ERROR some.test FAILED (TIMEOUT)"]
        )

    def testDistinctTestsBothKept(self):
        lines = ["ERROR a FAILED (X)", "ERROR b FAILED (Y)"]
        self.assertEqual(sorted(cpf.dedupe_test_failures(lines)), sorted(lines))


class TestTestNameOf(unittest.TestCase):
    def testNameAfterStatusWord(self):
        line = (
            "ERROR controls/web_server_control.connect_port FAILED (EXIT CODE 1 != 0)"
        )
        self.assertEqual(
            cpf.test_name_of(line), "controls/web_server_control.connect_port"
        )


class TestFailingSteps(unittest.TestCase):
    def testOnlyNonzeroReturnCodesSelected(self):
        steps = [
            ("01_Build", "...\ncompleted with return code 0\n"),
            ("02_Test", "...\ncompleted with return code 1\n"),
        ]
        self.assertEqual(cpf.failing_steps(steps), [("02_Test", 1, steps[1][1])])

    def testLastReturnCodeWinsWhenStepRetried(self):
        text = (
            "completed with return code 1\n...retry...\ncompleted with return code 85\n"
        )
        self.assertEqual(cpf.failing_steps([("step", text)]), [("step", 85, text)])

    def testNoReturnCodeMeansNotFailing(self):
        self.assertEqual(cpf.failing_steps([("step", "no marker here")]), [])


class TestExtractStepErrors(unittest.TestCase):
    def testTestKindCollectsFailuresAndTally(self):
        text = (
            "[0.1s] [ 10MB] some/test: ERROR some.test FAILED (TIMEOUT)\n"
            "1 passed, 1 FAILED\n"
        )
        result = cpf.extract_step_errors(text, limit=5)
        self.assertEqual(result["kind"], "test")
        self.assertEqual(result["total"], 1)
        self.assertEqual(result["tally"], "1 passed, 1 FAILED")
        self.assertIn("TIMEOUT", result["reasons"][0])

    def testBuildKindFromCompilerDiagnostic(self):
        text = "foo.cc:12:3: error: use of undeclared identifier 'x'\n"
        result = cpf.extract_step_errors(text, limit=5)
        self.assertEqual(result["kind"], "build")
        self.assertEqual(result["total"], 1)

    def testBuildDiagnosticPreemptsMakeFailureSymptom(self):
        text = (
            "foo.cc:12:3: error: use of undeclared identifier 'x'\n"
            "make[1]: *** [foo.o] Error 1\n"
        )
        result = cpf.extract_step_errors(text, limit=5)
        self.assertEqual(result["kind"], "build")
        # The TestHarness-prefix stripping above also strips a "file:line:col: "
        # compiler prefix, since it has the same shape.
        self.assertEqual(result["items"], ["error: use of undeclared identifier 'x'"])

    def testInfraKindWinsOverNonDiagnosticBuildComplaint(self):
        text = "ninja: error: loading 'build.ninja'\ncurl: (28) connection timed out\n"
        result = cpf.extract_step_errors(text, limit=5)
        self.assertEqual(result["kind"], "infra")

    def testMakeFailureFallsBackToBuildWhenNothingElseMatches(self):
        text = "make[2]: *** [target] Error 2\n"
        result = cpf.extract_step_errors(text, limit=5)
        self.assertEqual(result["kind"], "build")

    def testUnknownKindFallsBackToTailWithoutTeardown(self):
        text = "useful line one\nuseful line two\nRemoving container\nSubmitting step statistics\n"
        result = cpf.extract_step_errors(text, limit=5)
        self.assertEqual(result["kind"], "unknown")
        self.assertEqual(result["items"], ["useful line one", "useful line two"])

    def testLimitCapsItemsButNotTotal(self):
        lines = "\n".join(f"error: problem {i}" for i in range(10))
        result = cpf.extract_step_errors(lines, limit=3)
        self.assertEqual(len(result["items"]), 3)
        self.assertEqual(result["total"], 10)


class TestRollupEntries(unittest.TestCase):
    def testShortestCommandChosenWithMatchingContainer(self):
        rollup = {
            "a.test": {
                "status": "ERROR",
                "reasons": {"TIMEOUT"},
                "failures": [
                    cpf.Failure(
                        job="job1",
                        command="cd test && ./run_tests --re a",
                        mode="-p 4",
                        container="docker://x",
                    ),
                    cpf.Failure(
                        job="job2",
                        command="./run_tests --re a",
                        mode="-p 8",
                        container=None,
                    ),
                ],
            },
        }
        entries = cpf.rollup_entries(rollup, limit=10)
        self.assertEqual(len(entries), 1)
        entry = entries[0]
        self.assertEqual(entry["test"], "a.test")
        self.assertEqual(entry["jobs"], ["job1", "job2"])
        self.assertEqual(entry["modes"], 2)
        self.assertEqual(entry["reproduce"], "./run_tests --re a")
        self.assertIsNone(entry["container"])

    def testLimitCapsEntries(self):
        failure = cpf.Failure(
            job="job1", command="./run_tests --re x", mode="", container=None
        )
        rollup = {
            f"t{i}.test": {"status": "ERROR", "reasons": set(), "failures": [failure]}
            for i in range(5)
        }
        self.assertEqual(len(cpf.rollup_entries(rollup, limit=2)), 2)


class TestGreppedLines(unittest.TestCase):
    def testUnionOfMatchesWithContext(self):
        body = [
            "one",
            "two",
            "MATCH here",
            "four",
            "five",
            "six",
            "MATCH again",
            "eight",
        ]
        self.assertEqual(cpf.grepped_lines(body, "MATCH", 1), [1, 2, 3, 5, 6, 7])

    def testNoMatchIsEmpty(self):
        self.assertEqual(cpf.grepped_lines(["a", "b"], "nope", 0), [])


def cfg_file(directory, name, display_name, requires=()):
    """Write a minimal CIVET recipe .cfg with an optional dependency list."""
    lines = ["[Main]", f"display_name = {display_name}", ""]
    if requires:
        lines += ["[PullRequest Dependencies]"]
        lines += [f"filename{i + 1} = {r}" for i, r in enumerate(requires)]
    with open(os.path.join(directory, name), "w") as handle:
        handle.write("\n".join(lines) + "\n")


class TestLoadRecipeGraphAndDownstream(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.mkdtemp()
        cfg_file(self.tmp, "build.cfg", "Build")
        cfg_file(self.tmp, "test.cfg", "Test", requires=["build.cfg"])
        cfg_file(self.tmp, "other.cfg", "Other")

    def testGraphMapsNamesAndDeps(self):
        graph = cpf.load_recipe_graph(self.tmp)
        self.assertEqual(
            graph["files_of"],
            {
                "Build": {"build.cfg"},
                "Test": {"test.cfg"},
                "Other": {"other.cfg"},
            },
        )
        self.assertEqual(graph["deps"]["test.cfg"], ["build.cfg"])

    def testDownstreamIsTransitiveDependent(self):
        graph = cpf.load_recipe_graph(self.tmp)
        self.assertEqual(cpf.downstream_of(graph, ["Build"]), {"Test"})

    def testBlockedPendingNeedsFailureAndGraph(self):
        graph = cpf.load_recipe_graph(self.tmp)
        statuses = [
            {"context": "Build", "state": "failure", "description": "x"},
            {"context": "Test", "state": "pending"},
            {"context": "Other", "state": "pending"},
        ]
        self.assertEqual(cpf.blocked_pending(graph, statuses), ["Test"])
        self.assertEqual(cpf.blocked_pending(None, statuses), [])

    def testEmptyDirWarnsAndReturnsNone(self):
        empty = tempfile.mkdtemp()
        with mock.patch("sys.stderr", new_callable=io.StringIO):
            self.assertIsNone(cpf.load_recipe_graph(empty))


TEST_STEP_TEXT = (
    "Executing step in docker://img:tag\n"
    "[00:00:01] test: ./run_tests -p 4\n"
    "[0.1s] [ 10MB] some/test: ERROR some.test FAILED (TIMEOUT)\n"
    "1 passed, 1 FAILED\n"
    "completed with return code 1\n"
)


class TestCollectJobErrors(unittest.TestCase):
    def testTestStepPopulatesStepsAndRollup(self):
        steps = [
            ("01_Build", "completed with return code 0\n"),
            ("02_Test", TEST_STEP_TEXT),
        ]
        rollup, hint_texts = {}, set()
        entry = cpf.collect_job_errors(
            "ctx",
            "http://job/1",
            steps,
            rollup,
            hint_texts,
            max_diagnostics=5,
            max_signatures=5,
        )
        self.assertEqual(len(entry["steps"]), 1)
        step = entry["steps"][0]
        self.assertEqual(step["kind"], "test")
        self.assertEqual(step["tests_failed"], 1)
        self.assertEqual(step["container"], "docker://img:tag")
        self.assertIn("some.test", rollup)
        failure = rollup["some.test"]["failures"][0]
        self.assertEqual(
            failure.command, "cd test && ./run_tests -p 4 --re '^some\\.test$'"
        )

    def testNoFailingStepReportsError(self):
        entry = cpf.collect_job_errors(
            "ctx", "url", [("step", "completed with return code 0\n")], {}, set(), 5, 5
        )
        self.assertIn("error", entry)


class TestCollectOneJob(unittest.TestCase):
    def testUsesRecipeNameFromEnv(self):
        steps = [("01_Build", "completed with return code 1\nerror: something broke\n")]
        result = cpf.collect_one_job(
            "http://job/2", steps, {"CIVET_RECIPE_NAME": "My Recipe"}, 5, 5
        )
        self.assertEqual(result["jobs"][0]["context"], "My Recipe")
        self.assertEqual(
            result["jobs"][0]["steps"][0]["diagnostics"], ["error: something broke"]
        )
        self.assertEqual(result["jobs_not_read"], 0)


class TestCollectErrors(unittest.TestCase):
    def testFetchesEachFailedJobUpToMaxLogJobs(self):
        failed_jobs = [
            {"context": f"job{i}", "target_url": f"http://job/{i}"} for i in range(3)
        ]
        with mock.patch.object(
            cpf, "fetch_job_steps", return_value=[("step", TEST_STEP_TEXT)]
        ):
            collected = cpf.collect_errors(
                failed_jobs, max_jobs=2, max_diagnostics=5, max_signatures=5
            )
        self.assertEqual(len(collected["jobs"]), 2)
        self.assertEqual(collected["jobs_not_read"], 1)
        self.assertIn("some.test", collected["rollup"])

    def testUnreadableLogIsReportedNotRaised(self):
        failed_jobs = [{"context": "job0", "target_url": "http://job/0"}]
        with mock.patch.object(cpf, "fetch_job_steps", side_effect=RuntimeError("403")):
            collected = cpf.collect_errors(
                failed_jobs, max_jobs=5, max_diagnostics=5, max_signatures=5
            )
        self.assertIn("could not read logs", collected["jobs"][0]["error"])


SAMPLE_INFO = {"number": 42, "headRefName": "br", "headRefOid": "abc123def456"}
SAMPLE_STATE = {
    "state": "failure",
    "statuses": [
        {"context": "A", "state": "failure", "description": "boom", "target_url": "u1"},
        {"context": "B", "state": "success", "description": None, "target_url": "u2"},
        {"context": "C", "state": "pending", "description": None, "target_url": "u3"},
    ],
}


def captured(func, *args, **kwargs):
    buf = io.StringIO()
    with redirect_stdout(buf):
        func(*args, **kwargs)
    return buf.getvalue()


class TestJsonPayload(unittest.TestCase):
    def testCountsAndJobListExcludeSuccess(self):
        collected = {"jobs": [], "rollup": {}, "hints": [], "jobs_not_read": 0}
        payload = cpf.json_payload(
            SAMPLE_INFO, SAMPLE_STATE, collected, max_failures=10
        )
        self.assertEqual(
            payload["counts"],
            {
                "jobs": 3,
                "failed": 1,
                "blocked": 0,
                "pending": 1,
                "pending_blocked": 0,
                "passed": 1,
            },
        )
        self.assertEqual([j["context"] for j in payload["jobs"]], ["A", "C"])
        self.assertFalse(payload["complete"])


class TestPrintDigest(unittest.TestCase):
    def testReportsCountsAndIncompleteWarning(self):
        text = captured(cpf.print_digest, SAMPLE_INFO, SAMPLE_STATE)
        self.assertIn("PR #42 br @ abc123d", text)
        self.assertIn("1 failed, 1 pending, 1 passed / 3", text)
        self.assertIn("EVENT INCOMPLETE", text)
        self.assertIn("A  u1", text)


class TestPrintJobHeader(unittest.TestCase):
    def testKnownFieldsPrintedUnknownFieldsAreQuestionMarks(self):
        env = {
            "CIVET_RECIPE_NAME": "My Recipe",
            "CIVET_PR_NUM": "99",
            "CIVET_EVENT_CAUSE": "Pull request",
        }
        text = captured(cpf.print_job_header, "http://job/2", env)
        self.assertIn("My Recipe  http://job/2", text)
        self.assertIn("pull request 99 (Pull request)", text)
        self.assertIn("head ?:? @ ?", text)
        self.assertIn("base ?:? @ ?", text)


class TestPrintErrorsAndSteps(unittest.TestCase):
    def setUp(self):
        self.steps = [
            ("01_Build", "completed with return code 1\nerror: something broke\n")
        ]
        self.env = {"CIVET_RECIPE_NAME": "My Recipe"}

    def testPrintErrorsShowsDiagnostics(self):
        collected = cpf.collect_one_job("http://job/2", self.steps, self.env, 5, 5)
        text = captured(cpf.print_errors, collected, max_failures=10, label_jobs=False)
        self.assertIn("01_Build (exit 1, build)", text)
        self.assertIn("error: something broke", text)

    def testPrintStepListShowsSizeInKB(self):
        text = captured(cpf.print_step_list, "http://job/2", self.steps)
        self.assertIn("steps in http://job/2", text)
        self.assertIn("01_Build", text)

    def testPrintStepLogTailWhenNoPattern(self):
        text = captured(cpf.print_step_log, self.steps, "01_Build", 100, None, 0)
        self.assertIn("last 2 of 2 lines", text)
        self.assertIn("error: something broke", text)

    def testPrintStepLogGrepsAndMarksGaps(self):
        text = captured(cpf.print_step_log, self.steps, "01_Build", 100, "broke", 0)
        self.assertIn("1 of 2 lines match 'broke'", text)
        self.assertIn("error: something broke", text)

    def testPrintStepLogUnknownStepRaises(self):
        with self.assertRaises(cpf.GitHubError):
            cpf.print_step_log(self.steps, "no-such-step", 100, None, 0)


def completed(returncode, stdout="", stderr=""):
    return subprocess.CompletedProcess(
        args=[], returncode=returncode, stdout=stdout, stderr=stderr
    )


class TestGh(unittest.TestCase):
    def testSuccessReturnsStdout(self):
        with mock.patch("subprocess.run", return_value=completed(0, "hello\n")):
            self.assertEqual(cpf.gh(["foo"]), "hello\n")

    def testNonZeroExitRaisesGitHubError(self):
        with mock.patch("subprocess.run", return_value=completed(1, "", "boom")):
            with self.assertRaisesRegex(cpf.GitHubError, "boom"):
                cpf.gh(["foo"])

    def testMissingGhRaisesGitHubError(self):
        with mock.patch("subprocess.run", side_effect=FileNotFoundError()):
            with self.assertRaisesRegex(cpf.GitHubError, "not found on PATH"):
                cpf.gh(["foo"])

    def testGhJsonParsesOrReturnsNoneWhenEmpty(self):
        with mock.patch.object(cpf, "gh", return_value='{"a": 1}\n'):
            self.assertEqual(cpf.gh_json(["x"]), {"a": 1})
        with mock.patch.object(cpf, "gh", return_value=""):
            self.assertIsNone(cpf.gh_json(["x"]))


class TestCurrentBranchAndRepoFromRemote(unittest.TestCase):
    def testCurrentBranch(self):
        with mock.patch("subprocess.run", return_value=completed(0, "mybranch\n")):
            self.assertEqual(cpf.current_branch(), "mybranch")

    def testCurrentBranchNotAGitRepoRaises(self):
        with mock.patch("subprocess.run", return_value=completed(1)):
            with self.assertRaises(cpf.GitHubError):
                cpf.current_branch()

    def testRepoFromRemoteParsesSlug(self):
        with mock.patch(
            "subprocess.run",
            return_value=completed(0, "git@github.com:idaholab/moose.git\n"),
        ):
            self.assertEqual(cpf.repo_from_remote("origin"), "idaholab/moose")

    def testRepoFromRemoteMissingRemoteReturnsNone(self):
        with mock.patch("subprocess.run", return_value=completed(1)):
            self.assertIsNone(cpf.repo_from_remote("nope"))


class TestResolveRepo(unittest.TestCase):
    def testExplicitRepoWins(self):
        self.assertEqual(cpf.resolve_repo("owner/name", None), "owner/name")

    def testNamedRemoteUsedBeforeCandidates(self):
        with mock.patch.object(
            cpf, "repo_from_remote", return_value="owner/fromremote"
        ):
            self.assertEqual(cpf.resolve_repo(None, "up"), "owner/fromremote")

    def testCandidatesTriedInOrderUntilOneHits(self):
        seen = []

        def fake(remote):
            seen.append(remote)
            return "owner/x" if remote == "upstream" else None

        with mock.patch.object(cpf, "repo_from_remote", side_effect=fake):
            self.assertEqual(cpf.resolve_repo(None, None), "owner/x")
        self.assertEqual(seen, ["up", "upstream"])

    def testNoneFoundReturnsNone(self):
        with mock.patch.object(cpf, "repo_from_remote", return_value=None):
            self.assertIsNone(cpf.resolve_repo(None, None))


class TestResolvePrAndTarget(unittest.TestCase):
    def testExplicitPrResolved(self):
        with mock.patch.object(cpf, "gh_json", return_value={"number": 5}):
            self.assertEqual(cpf.resolve_pr("o/n", 5), {"number": 5})

    def testExplicitPrNotFoundRaises(self):
        with mock.patch.object(cpf, "gh_json", return_value=None):
            with self.assertRaises(cpf.GitHubError):
                cpf.resolve_pr("o/n", 5)

    def testDefaultsToBranchsOpenPr(self):
        with (
            mock.patch.object(cpf, "current_branch", return_value="mybr"),
            mock.patch.object(cpf, "gh_json", return_value=[{"number": 7}]),
        ):
            self.assertEqual(cpf.resolve_pr("o/n", None), {"number": 7})

    def testNoOpenPrForBranchRaises(self):
        with (
            mock.patch.object(cpf, "current_branch", return_value="mybr"),
            mock.patch.object(cpf, "gh_json", return_value=[]),
        ):
            with self.assertRaises(cpf.GitHubError):
                cpf.resolve_pr("o/n", None)

    def testResolveTargetShaSkipsPrLookup(self):
        info = cpf.resolve_target("o/n", None, "deadbeef")
        self.assertEqual(info["headRefOid"], "deadbeef")
        self.assertIsNone(info["number"])

    def testResolveTargetPrDelegatesToResolvePr(self):
        with mock.patch.object(cpf, "resolve_pr", return_value={"number": 1}):
            self.assertEqual(cpf.resolve_target("o/n", 1, None), {"number": 1})


class FakeUrlResponse:
    """Minimal context-manager stand-in for urllib.request.urlopen's result."""

    def __init__(self, data):
        self.data = data

    def read(self):
        return self.data

    def __enter__(self):
        return self

    def __exit__(self, *exc):
        return False


class TestFetchState(unittest.TestCase):
    def testCombinesRollupStateAndPaginatedStatuses(self):
        status_line = json.dumps({"context": "A", "state": "failure"}) + "\n"
        with (
            mock.patch.object(cpf, "gh_json", return_value={"state": "failure"}),
            mock.patch.object(cpf, "gh", return_value=status_line),
        ):
            state = cpf.fetch_state("o/n", "sha")
        self.assertEqual(state["state"], "failure")
        self.assertEqual(state["statuses"], [{"context": "A", "state": "failure"}])


class TestFetchJobSteps(unittest.TestCase):
    def testExtractsEachTarMemberAsAStep(self):
        buf = io.BytesIO()
        with tarfile.open(fileobj=buf, mode="w:gz") as tar:
            data = b"hello world\n"
            info = tarfile.TarInfo(name="job/01_Build")
            info.size = len(data)
            tar.addfile(info, io.BytesIO(data))

        with mock.patch(
            "urllib.request.urlopen", return_value=FakeUrlResponse(buf.getvalue())
        ):
            steps = cpf.fetch_job_steps("https://civet.inl.gov/job/123/")
        self.assertEqual(steps, [("01_Build", "hello world\n")])


class TestParseArgs(unittest.TestCase):
    def testDefaults(self):
        args = cpf.parse_args([])
        self.assertIsNone(args.pr)
        self.assertIsNone(args.sha)
        self.assertIsNone(args.job)
        self.assertEqual(args.max_failures, cpf.DEFAULT_MAX_FAILURES)
        self.assertFalse(args.as_json)

    def testPrAndShaAreMutuallyExclusive(self):
        with mock.patch("sys.stderr", new_callable=io.StringIO):
            with self.assertRaises(SystemExit):
                cpf.parse_args(["--pr", "5", "--sha", "abc"])


JOB_STEPS = [("01_Build", "completed with return code 1\nerror: something broke\n")]


class TestMainJobDispatch(unittest.TestCase):
    def testListSteps(self):
        with mock.patch.object(cpf, "fetch_job_steps", return_value=JOB_STEPS):
            text = captured(cpf.main, ["--job", "http://job/2", "--list-steps"])
        self.assertIn("steps in http://job/2", text)

    def testStep(self):
        with mock.patch.object(cpf, "fetch_job_steps", return_value=JOB_STEPS):
            text = captured(cpf.main, ["--job", "http://job/2", "--step", "01_Build"])
        self.assertIn("error: something broke", text)

    def testPlainJobReportsHeaderAndErrors(self):
        with mock.patch.object(cpf, "fetch_job_steps", return_value=JOB_STEPS):
            text = captured(cpf.main, ["--job", "http://job/2"])
        self.assertIn("http://job/2", text)
        self.assertIn("error: something broke", text)

    def testUnreadableJobRaisesGitHubError(self):
        with mock.patch.object(cpf, "fetch_job_steps", side_effect=RuntimeError("403")):
            with self.assertRaises(cpf.GitHubError):
                cpf.main(["--job", "http://job/2"])


class TestMainPrDispatch(unittest.TestCase):
    def setUp(self):
        self.info = {"number": 5, "headRefName": "br", "headRefOid": "deadbeef123"}
        self.state = {"state": "success", "statuses": []}

    def testDigestOnlyWhenNoErrorsOrJson(self):
        with (
            mock.patch.object(cpf, "resolve_repo", return_value="o/n"),
            mock.patch.object(cpf, "resolve_target", return_value=self.info),
            mock.patch.object(cpf, "fetch_state", return_value=self.state),
            mock.patch.object(cpf, "collect_errors") as collect,
        ):
            text = captured(cpf.main, ["--pr", "5"])
        collect.assert_not_called()
        self.assertIn("PR #5 br @ deadbee", text)

    def testJsonImpliesErrorsAndEmitsValidJson(self):
        collected = {"jobs": [], "rollup": {}, "hints": [], "jobs_not_read": 0}
        with (
            mock.patch.object(cpf, "resolve_repo", return_value="o/n"),
            mock.patch.object(cpf, "resolve_target", return_value=self.info),
            mock.patch.object(cpf, "fetch_state", return_value=self.state),
            mock.patch.object(cpf, "collect_errors", return_value=collected) as collect,
        ):
            text = captured(cpf.main, ["--pr", "5", "--json"])
        collect.assert_called_once()
        self.assertEqual(json.loads(text)["pr"], 5)

    def testNoRepoFoundRaisesGitHubError(self):
        with mock.patch.object(cpf, "resolve_repo", return_value=None):
            with self.assertRaises(cpf.GitHubError):
                cpf.main([])


if __name__ == "__main__":
    unittest.main()
