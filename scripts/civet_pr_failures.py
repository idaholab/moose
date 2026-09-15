#!/usr/bin/env python3
"""
Report the CIVET failures for a pull request in a compact, bounded form.

Reads only through the GitHub API (via the gh CLI), so it works from a
machine that can reach github.com. Two sources are combined:

  - The commit statuses CIVET posts per job, which give job-level state and
    the URL of each CIVET job.
  - The machine-readable block that the test summary comment carries, which
    gives the individual failing tests and how to reproduce them.

The output is deliberately small. Passing tests are never reported, failures
already present in the base are reported as a count rather than a list, and
every list is capped. Use --json for the merged payload.
"""

# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

import argparse
import collections
import configparser
import glob
import json
import os
import re
import subprocess
import sys
from typing import List, Optional, Sequence

# Remotes tried, in order, when none is named. A developer checkout usually
# has origin pointing at a personal fork rather than the upstream, so origin
# is the last resort. Override with --upstream-remote, --repo, or the
# CIVET_UPSTREAM_REMOTE environment variable.
UPSTREAM_REMOTE_CANDIDATES = ("up", "upstream", "origin")

# Default cap on the number of failing tests to describe
DEFAULT_MAX_FAILURES = 20

# Default cap on the number of failed jobs to list
MAX_JOBS = 15

# Status state CIVET posts for a job that has not finished. GitHub's combined
# state cannot be used to tell whether an event is done: it turns "failure" as
# soon as any one job fails, with every other job still pending.
PENDING_STATE = "pending"

# Description CIVET sets on a job that was skipped because something it
# depends on failed. These are cascade effects, not failures to fix.
BLOCKED_DESCRIPTION = "Won't run due to failed dependencies"

# Every CIVET step log ends with this, which is how a failing step is found
# without asking CIVET for per-step status
STEP_RETURN_CODE_RE = re.compile(r"completed with return code (\d+)")

# Leading timestamp and memory annotations on TestHarness output lines
LINE_PREFIX_RE = re.compile(r"^\[[\d.]+s\]\s*\[\s*\d+MB\]\s*")

# The TestHarness prefixes each line of a test's output with the test name.
# It has to come off before output lines can be compared, or every line looks
# unique. Only a token shaped like a test name is stripped, so that a stack
# frame such as "0: libMesh::print_trace" keeps its leading label.
TEST_PREFIX_RE = re.compile(r"^[\w\-./:]*[./][\w\-./:]*:[ ]?")

# A compiler or linker diagnostic. The make failure lines that follow are
# noise once the diagnostic itself is in hand, so they are only a fallback.
DIAGNOSTIC_RE = re.compile(r"\berror:|\bundefined reference to\b|^ld: ")
MAKE_FAILURE_RE = re.compile(r"\*\*\* \[.*\] Error \d+")

# Failures from the surrounding machinery rather than from compiled code or a
# test: a container build, a download, or a script the recipe drives. These
# carry no compiler diagnostic, so they need their own patterns.
INFRA_ERROR_RE = re.compile(
    r"^FATAL:|curl: \(\d+\)|CalledProcessError|Connection reset by peer|"
    r"Could not resolve host|No space left on device|Connection timed out|"
    r"^fatal: unable to access|^fatal: remote error|Empty reply from server|"
    r"RPC failed"
)

# Lines that match the diagnostic pattern while only reporting someone else's
# failure. A build system saying it cannot find its own generated makefile, or
# git reporting a transport error, says nothing about the cause, so these must
# not preempt the infra check that finds it.
NON_DIAGNOSTIC_RE = re.compile(r"^(?:ninja|make|gmake)(?:\[\d+\])?: |^fatal: ")

# Step teardown, which is the last thing in every log and says nothing about
# why the step failed. It has to be dropped before falling back to the tail.
TEARDOWN_MARKERS = (
    "Removing ",
    "rm -rf",
    "Submitting step statistics",
    "Failed to submit step statistics",
    "Execution in ",
    "Collecting test statistics",
    "ERROR: Exiting with code",
)

# A TestHarness failure line, and its end-of-run tally
TEST_FAILURE_RE = re.compile(r"\bFAILED \(")
TEST_TALLY_RE = re.compile(r"^\d+ passed, .*\bFAILED\b")

# The literal harness invocation a step logged, as "[time] <dir>: ./run_tests ..."
INVOCATION_RE = re.compile(r"^\[[\d:]+\]\s+(\S+):\s+(\./run_tests\b.*)$")

# Arguments that describe the CI machine rather than the test mode, and so are
# dropped from a reproduce command. Everything else is kept, including
# --compute-device and --max-memory-per-slot, which change whether tests pass.
STRIP_EXACT_ARGS = {"-t", "--timing"}
STRIP_PREFIX_ARGS = ("--longest-jobs=",)
STRIP_VALUED_ARGS = {"-j", "-l"}

# Arguments kept in a reproduce command but ignored when deciding whether two
# runs were the same test mode. Memory limits are tuned per job and would
# otherwise make one mode look like several.
MODE_EXCLUDE_PREFIXES = ("--max-memory-per-slot=",)

# A MOOSE error block opens with this marker and carries boilerplate before
# the message that identifies the failure
ERROR_BLOCK_MARKER = "*** ERROR ***"
ERROR_BOILERPLATE = ("The following occurred",)

# Errors that stand on their own line, such as those a Python tester raises
STANDALONE_ERROR_RE = re.compile(r"^(?:[A-Za-z_]*Error|Assertion|Fatal error)[:\s]")

# Applied before comparing two error messages, so that a differing temporary
# directory or index does not make one failure look like many
NORMALIZE_SUBS = (
    (re.compile(r"/tmp/[^/\s]+"), "/tmp/<dir>"),
    (re.compile(r"0x[0-9a-fA-F]+"), "<addr>"),
    (re.compile(r"\d+"), "N"),
)

# Known remediations, tried in order against a failure reason or error
# signature. Each is tied to the tester that produces the status:
# EXPECTED OUTPUT MISSING comes from RunApp's expect_out, while EXPECTED
# OUTPUT NOT FOUND comes only from PetscJacobianTester and TaoGradientTester,
# which look for a specific diagnostic line rather than comparing gold files.
REMEDIATION_HINTS = (
    (
        "KILLED: OVER MEMORY",
        "raise min_slots in the test spec; not reproducible locally",
    ),
    (
        "Error opening ExodusII mesh file: /tmp/",
        "the recipe copied tests without their mesh files; likely spurious, "
        "not a code defect",
    ),
    (
        "EXPECTED OUTPUT MISSING",
        "the test's expect_out pattern did not match the program output; "
        "update expect_out, or fix the output it describes",
    ),
    (
        "EXPECTED OUTPUT NOT FOUND",
        "a Jacobian or gradient quality test could not find its diagnostic "
        "line, so the run usually died before PETSc/TAO printed it; look for "
        "the real error earlier in the same output",
    ),
    (
        "curl: (",
        "a download failed during the build; spurious, re-run the job rather "
        "than changing code",
    ),
    (
        "fatal: unable to access",
        "a git clone or fetch failed during the build; spurious, re-run the job",
    ),
    (
        "fatal: remote error",
        "the ref being fetched is not on the remote, usually a submodule bumped "
        "to a commit that has not been pushed upstream; push it, then re-run",
    ),
    (
        "FATAL:",
        "the container build itself failed; check the step log, and suspect "
        "infrastructure before code",
    ),
    (
        "EXPECTED ERROR MISSING",
        "the error text the test expects has changed; update expect_err",
    ),
)

# Default cap on error signatures reported per job
DEFAULT_MAX_SIGNATURES = 4

# Default cap on unique diagnostics reported per failing step
DEFAULT_MAX_DIAGNOSTICS = 5

# Default cap on jobs to pull logs for, since each is a tarball download
DEFAULT_MAX_LOG_JOBS = 6


class GitHubError(SystemExit):
    """Exception for a failed gh invocation."""


def gh(args: Sequence[str]) -> str:
    """Run a gh command and return its stdout."""
    try:
        result = subprocess.run(
            ["gh", *args], capture_output=True, text=True, check=False
        )
    except FileNotFoundError as e:
        raise GitHubError("The gh CLI is required and was not found on PATH") from e
    if result.returncode != 0:
        raise GitHubError(f"gh {' '.join(args)} failed:\n{result.stderr.strip()}")
    return result.stdout


def gh_json(args: Sequence[str]):
    """Run a gh command that produces JSON and parse it."""
    out = gh(args).strip()
    return json.loads(out) if out else None


def latest_statuses(statuses: Sequence[dict]) -> List[dict]:
    """
    Reduce the status list to the newest status per context.

    GitHub returns every status ever posted for a commit, newest first, and
    CIVET posts one when a job starts and again when it finishes. Taking the
    first occurrence of each context avoids reporting a finished job as
    pending.
    """
    seen = set()
    latest = []
    for status in statuses:
        context = status.get("context")
        if context in seen:
            continue
        seen.add(context)
        latest.append(status)
    return latest


def current_branch() -> str:
    """Get the name of the checked out branch."""
    result = subprocess.run(
        ["git", "rev-parse", "--abbrev-ref", "HEAD"],
        capture_output=True,
        text=True,
        check=False,
    )
    if result.returncode != 0:
        raise GitHubError("Not in a git repository; pass --pr")
    return result.stdout.strip()


def repo_from_remote(remote: str) -> Optional[str]:
    """Get the owner/name slug named by a git remote, if it exists."""
    result = subprocess.run(
        ["git", "remote", "get-url", remote],
        capture_output=True,
        text=True,
        check=False,
    )
    if result.returncode != 0:
        return None
    match = re.search(r"[:/]([^/:]+)/([^/]+?)(?:\.git)?$", result.stdout.strip())
    return f"{match.group(1)}/{match.group(2)}" if match else None


def resolve_repo(repo: Optional[str], remote: Optional[str]) -> Optional[str]:
    """
    Decide which repository to query.

    Precedence: an explicit --repo, then a named remote (--upstream-remote or
    CIVET_UPSTREAM_REMOTE), then each candidate remote in turn.
    """
    if repo:
        return repo
    named = remote or os.environ.get("CIVET_UPSTREAM_REMOTE")
    for candidate in [named] if named else UPSTREAM_REMOTE_CANDIDATES:
        if slug := repo_from_remote(candidate):
            return slug
    return None


def resolve_pr(repo: str, pr: Optional[int]) -> dict:
    """
    Resolve the PR number and head SHA, defaulting to the branch's PR.

    The repository is always passed explicitly. A MOOSE checkout commonly has
    a remote per fork whose PRs are reviewed locally, and gh picks one of them
    arbitrarily when it has no default, which resolves to the wrong repository
    or to no pull request at all.
    """
    fields = "number,headRefOid,headRefName,url"

    if pr is not None:
        info = gh_json(["pr", "view", str(pr), "--repo", repo, "--json", fields])
        if not info:
            raise GitHubError(f"Could not resolve PR #{pr} in {repo}")
        return info

    branch = current_branch()
    found = gh_json(
        [
            "pr",
            "list",
            "--repo",
            repo,
            "--head",
            branch,
            "--json",
            fields,
            "--limit",
            "1",
        ]
    )
    if not found:
        raise GitHubError(
            f"No open pull request in {repo} for branch '{branch}'; pass --pr"
        )
    return found[0]


def fetch_state(slug: str, sha: str) -> dict:
    """
    Fetch the rollup state and every per-job status for a SHA.

    The combined status endpoint caps its statuses array at 30, which silently
    truncates an event with more jobs than that, so the per-job statuses come
    from the paginated list endpoint instead. Its rollup state is still taken
    from the combined endpoint, where it accounts for every job.
    """
    combined = gh_json(["api", f"repos/{slug}/commits/{sha}/status"]) or {}

    # --jq emits one object per line, avoiding the concatenated arrays that
    # --paginate produces on its own
    raw = gh(
        [
            "api",
            f"repos/{slug}/commits/{sha}/statuses",
            "--paginate",
            "--jq",
            ".[]",
        ]
    )
    statuses = [json.loads(line) for line in raw.splitlines() if line.strip()]

    return {
        "state": combined.get("state", "unknown"),
        "statuses": latest_statuses(statuses),
    }


def fetch_job_steps(job_url: str) -> List[tuple]:
    """
    Fetch the step logs for a CIVET job as (name, text) pairs.

    CIVET serves every step of a job as a gzipped tarball with one member per
    step, which needs no authentication for public repositories.
    """
    import io
    import tarfile
    import urllib.request

    base = job_url.rstrip("/").replace("/job/", "/job_results/")
    with urllib.request.urlopen(f"{base}/") as response:
        payload = response.read()

    steps = []
    with tarfile.open(fileobj=io.BytesIO(payload), mode="r:gz") as tar:
        for member in tar.getmembers():
            handle = tar.extractfile(member)
            if handle is None:
                continue
            text = handle.read().decode("utf-8", "replace")
            steps.append((member.name.split("/")[-1], text))
    return steps


def unique(values: Sequence[str]) -> List[str]:
    """Deduplicate while preserving order."""
    seen = set()
    out = []
    for value in values:
        if value not in seen:
            seen.add(value)
            out.append(value)
    return out


def strip_ci_args(args: List[str]) -> List[str]:
    """Drop the arguments that only describe the CI machine."""
    kept = []
    skip_next = False
    for arg in args:
        if skip_next:
            skip_next = False
            continue
        if arg in STRIP_VALUED_ARGS:
            skip_next = True
            continue
        if arg in STRIP_EXACT_ARGS or arg.startswith(STRIP_PREFIX_ARGS):
            continue
        kept.append(arg)
    return kept


def extract_invocation(text: str) -> Optional[tuple]:
    """
    Get the (directory, args) of the harness invocation a step logged.

    The first invocation is the one wanted: a step that retried its failures
    logs a second one carrying --failed-tests.
    """
    for line in text.splitlines():
        match = INVOCATION_RE.match(line.strip())
        if match:
            directory = [p for p in match.group(1).split("/") if p]
            args = strip_ci_args(match.group(2).split()[1:])
            return (directory[-1] if directory else ".", args)
    return None


def reproduce_command(invocation: Optional[tuple], test: str) -> str:
    """
    Build the command that reruns one test in the mode that failed it.

    The mode arguments are taken from what the step actually ran rather than
    synthesized, since flags like --compute-device cannot be guessed from the
    step name and decide whether the test fails at all.
    """
    selector = f"--re '^{re.escape(test)}$'"
    if invocation is None:
        return f"./run_tests {selector}"
    directory, args = invocation
    prefix = f"cd {directory} && " if directory not in (".", "") else ""
    return f"{prefix}./run_tests {' '.join([*args, selector])}".replace("  ", " ")


def mode_identity(invocation: Optional[tuple]) -> str:
    """
    Identify the test mode of an invocation.

    Two runs count as the same mode when they differ only in resource limits,
    so that the reported mode count reflects the ways a test was exercised.
    """
    if invocation is None:
        return ""
    _, args = invocation
    return " ".join(a for a in args if not a.startswith(MODE_EXCLUDE_PREFIXES))


def retry_note(code: int) -> Optional[str]:
    """
    Describe what the retry did, from a test step's exit code.

    tests.sh reruns the failed tests when the first run exits between 1 and
    127, and reports 85 when that rerun passed. The exit code therefore says
    whether a failure survived a retry, which the stored results cannot.
    """
    if code == 85:
        return "passed on retry; intermittent"
    if 0 < code < 128:
        return "failed again on retry"
    return "no retry attempted"


def normalize_error(message: str) -> str:
    """Reduce an error message to a form two occurrences can be compared on."""
    for pattern, replacement in NORMALIZE_SUBS:
        message = pattern.sub(replacement, message)
    return message


def error_signatures(lines: Sequence[str]) -> "collections.Counter":
    """
    Count the distinct root errors in a step's output.

    A single defect usually surfaces in many tests, so the count of unique
    signatures says how many things actually went wrong, which the count of
    failing tests does not.
    """
    signatures: collections.Counter = collections.Counter()

    for i, line in enumerate(lines):
        message = None
        if ERROR_BLOCK_MARKER in line:
            for candidate in lines[i + 1 : i + 8]:
                candidate = candidate.strip()
                if not candidate or candidate.startswith(ERROR_BOILERPLATE):
                    continue
                message = candidate
                break
        elif STANDALONE_ERROR_RE.match(line.strip()):
            message = line.strip()

        if message:
            signatures[normalize_error(message)] += 1

    return signatures


def hints_for(texts: Sequence[str]) -> List[str]:
    """Get the remediations that apply to the given reasons and signatures."""
    hints = []
    for key, hint in REMEDIATION_HINTS:
        if any(key in text for text in texts):
            hints.append(f"{key} -> {hint}")
    return hints


def dedupe_test_failures(lines: Sequence[str]) -> List[str]:
    """
    Collapse the repeated reports of one failing test.

    The TestHarness prints a test again as it is finalized and again when the
    failed-test rerun repeats it, annotating each with a differing suffix, so
    the lines are deduplicated on the test they name and the least annotated
    one is kept.
    """
    best: dict = {}
    for line in lines:
        key = line.split(" FAILED (", 1)[0]
        if key not in best or len(line) < len(best[key]):
            best[key] = line
    return list(best.values())


def failing_steps(steps: Sequence[tuple]) -> List[tuple]:
    """
    Select the steps that exited nonzero, as (name, code, text).

    A job does not necessarily stop at its first failing step, and steps that
    run afterwards can succeed, so the last step is not a reliable indicator.
    """
    failing = []
    for name, text in steps:
        codes = STEP_RETURN_CODE_RE.findall(text)
        code = int(codes[-1]) if codes else None
        if code:
            failing.append((name, code, text))
    return failing


def extract_step_errors(text: str, limit: int) -> dict:
    """
    Extract the actionable lines from one failing step's log.

    Build and test failures need different treatment: a compiler diagnostic is
    repeated once per translation unit and must be deduplicated, while a test
    failure is one line per test plus an end-of-run tally.
    """
    lines = [
        TEST_PREFIX_RE.sub("", LINE_PREFIX_RE.sub("", line.rstrip()))
        for line in text.splitlines()
    ]

    test_failures = dedupe_test_failures(
        [line for line in lines if TEST_FAILURE_RE.search(line)]
    )
    if test_failures:
        tally = next(
            (line for line in reversed(lines) if TEST_TALLY_RE.match(line)), None
        )
        return {
            "kind": "test",
            "items": test_failures[:limit],
            "all_items": test_failures,
            "total": len(test_failures),
            "tally": tally,
            "signatures": error_signatures(lines),
            "reasons": [
                m.group(1)
                for m in (re.search(r"FAILED \(([^)]*)\)", f) for f in test_failures)
                if m
            ],
        }

    def entry(kind: str, items: List[str]) -> dict:
        return {
            "kind": kind,
            "items": items[:limit],
            "all_items": items,
            "total": len(items),
            "tally": None,
        }

    # A real compiler or linker diagnostic names the cause and wins outright
    diagnostics = unique(
        [
            line
            for line in lines
            if DIAGNOSTIC_RE.search(line) and not NON_DIAGNOSTIC_RE.match(line.strip())
        ]
    )
    if diagnostics:
        return entry("build", diagnostics)

    # Otherwise the surrounding machinery is where the cause is, and only then
    # the build system's own complaint about a failure it inherited
    infra = unique([line for line in lines if INFRA_ERROR_RE.search(line.strip())])
    if infra:
        return entry("infra", infra)

    symptoms = unique([line for line in lines if MAKE_FAILURE_RE.search(line)])
    if symptoms:
        return entry("build", symptoms)

    # Nothing recognized, so fall back to the tail, less the teardown that
    # every log ends with and that never explains the failure
    tail = [
        line
        for line in lines
        if line.strip() and not any(m in line for m in TEARDOWN_MARKERS)
    ][-limit:]
    return {
        "kind": "unknown",
        "items": tail,
        "all_items": tail,
        "total": len(tail),
        "tally": None,
    }


def test_name_of(line: str) -> str:
    """Get the test name out of a TestHarness failure line."""
    return line.split(" FAILED (", 1)[0].split(None, 1)[-1]


def rollup_entries(rollup: dict, limit: int) -> List[dict]:
    """
    Summarize the failing tests, one entry per test.

    The reproduce command is the shortest of the modes that failed, since a
    longer one only adds variables that were not needed to fail. The mode
    count is what says whether a failure is specific to one way of running.
    """
    entries = []
    for test, info in list(rollup.items())[:limit]:
        jobs = unique([job for job, _, _ in info["modes"]])
        commands = unique([command for _, command, _ in info["modes"]])
        modes = unique([mode for _, _, mode in info["modes"]])
        entries.append(
            {
                "test": test,
                "status": info["status"],
                "reasons": sorted(info.get("reasons", ())),
                "hints": hints_for(sorted(info.get("reasons", ()))),
                "jobs": jobs,
                "modes": len(modes),
                "reproduce": min(commands, key=len),
            }
        )
    return entries


def collect_errors(
    failed_jobs: Sequence[dict],
    max_jobs: int,
    max_diagnostics: int,
    max_signatures: int,
) -> dict:
    """
    Read the logs of each failed job and gather what they contain.

    Collection is kept separate from reporting so that the text digest and the
    JSON payload describe exactly the same findings.
    """
    jobs: List[dict] = []
    rollup: dict = {}
    hint_texts: set = set()

    for status in failed_jobs[:max_jobs]:
        entry = {
            "context": status.get("context") or "?",
            "url": status.get("target_url") or "",
            "steps": [],
        }
        jobs.append(entry)

        try:
            steps = fetch_job_steps(entry["url"])
        except Exception as e:  # noqa: BLE001 - a log we cannot read is not fatal
            entry["error"] = f"could not read logs: {e}"
            continue

        failing = failing_steps(steps)
        if not failing:
            entry["error"] = "no step reported a nonzero return code"
            continue

        for name, code, text in failing:
            errors = extract_step_errors(text, max_diagnostics)
            step = {"name": name, "exit_code": code, "kind": errors["kind"]}

            if errors["kind"] == "test":
                signatures = errors["signatures"]
                step["tests_failed"] = errors["total"]
                step["retry"] = retry_note(code)
                step["signatures_total"] = len(signatures)
                step["signatures"] = [
                    {"message": message, "count": count}
                    for message, count in signatures.most_common(max_signatures)
                ]
                hint_texts.update(signatures)

                invocation = extract_invocation(text)
                for line in errors["all_items"]:
                    test = test_name_of(line)
                    test_entry = rollup.setdefault(
                        test,
                        {
                            "status": line.split(None, 1)[0],
                            "modes": [],
                            "reasons": set(),
                        },
                    )
                    # The reason is what says how to act; the status word does
                    # not distinguish a memory kill from a genuine error
                    if reason := re.search(r"FAILED \(([^)]*)\)", line):
                        test_entry["reasons"].add(reason.group(1))
                    test_entry["modes"].append(
                        (
                            entry["context"],
                            reproduce_command(invocation, test),
                            mode_identity(invocation),
                        )
                    )
            else:
                step["diagnostics"] = errors["items"]
                step["diagnostics_total"] = errors["total"]
                hint_texts.update(errors["all_items"])

            entry["steps"].append(step)

    return {
        "jobs": jobs,
        "rollup": rollup,
        "hints": hints_for(sorted(hint_texts)),
        "jobs_not_read": max(0, len(failed_jobs) - max_jobs),
    }


def print_errors(collected: dict, max_failures: int) -> None:
    """Print the collected build diagnostics, error clusters and test rollup."""
    for job in collected["jobs"]:
        # A test step with no recognized error signature has nothing to add
        # here; its tests are reported in the rollup below
        shown = [
            step
            for step in job["steps"]
            if step["kind"] != "test" or step["signatures"]
        ]
        if not shown and "error" not in job:
            continue

        print(f"\n{job['context']}  {job['url']}")
        if error := job.get("error"):
            print(f"  {error}")

        for step in shown:
            if step["kind"] == "test":
                print(
                    f"  {step['name']} (exit {step['exit_code']}, "
                    f"{step['tests_failed']} tests failed, "
                    f"{step['signatures_total']} error signature(s), "
                    f"{step['retry']})"
                )
                for signature in step["signatures"]:
                    print(f"    x{signature['count']}  {signature['message'][:150]}")
                hidden = step["signatures_total"] - len(step["signatures"])
                if hidden:
                    print(f"    ... {hidden} more signature(s)")
            else:
                print(f"  {step['name']} (exit {step['exit_code']}, {step['kind']})")
                for item in step["diagnostics"]:
                    print(f"    {item[:200]}")
                hidden = step["diagnostics_total"] - len(step["diagnostics"])
                if hidden:
                    print(f"    ... {hidden} more unique")

    rollup = collected["rollup"]
    if rollup:
        print(f"\nunique failing tests: {len(rollup)}")
        for entry in rollup_entries(rollup, max_failures):
            label = ", ".join(entry["reasons"]) or entry["status"]
            print(f"  {entry['test']}  [{label}]")
            scope = (
                f"    failed in {len(entry['jobs'])} job(s), {entry['modes']} mode(s)"
            )
            if len(entry["jobs"]) <= 3:
                scope += f": {', '.join(entry['jobs'])}"
            print(scope)
            print(f"    {entry['reproduce']}")
            if entry["modes"] > 1:
                print(f"    (also failed in {entry['modes'] - 1} other mode(s))")
            for hint in entry["hints"]:
                print(f"    -> {hint.split(' -> ', 1)[-1]}")
        if len(rollup) > max_failures:
            print(f"  ... {len(rollup) - max_failures} more; raise --max-failures")

    if collected["hints"]:
        print("\nknown remediations (job level):")
        for hint in collected["hints"]:
            print(f"  {hint}")

    if collected["jobs_not_read"]:
        print(
            f"\n... {collected['jobs_not_read']} more failed jobs; "
            "raise --max-log-jobs"
        )


def print_job_log(job_url: str, step: Optional[str], lines: int) -> None:
    """
    Print a bounded tail of one step's log, or list the steps.

    Listing first keeps the caller from pulling a multi-megabyte log to find
    out which step it wanted.
    """
    steps = fetch_job_steps(job_url)

    if step is None:
        print(f"steps in {job_url} (pass --step NAME for a tail):")
        for name, text in steps:
            print(f"  {name}  ({len(text) / 1024:.0f} KB)")
        return

    matches = [(n, t) for n, t in steps if step in n]
    if not matches:
        available = ", ".join(n for n, _ in steps)
        raise GitHubError(f"No step matching '{step}'. Available: {available}")

    for name, text in matches:
        tail = text.splitlines()[-lines:]
        print(f"--- {name} (last {len(tail)} lines) ---")
        print("\n".join(tail))


def load_recipe_graph(recipes_dir: str) -> Optional[dict]:
    """
    Build the recipe dependency graph from a civet_recipes checkout.

    Entirely optional: CIVET's job statuses say nothing about dependencies, so
    a pending job that can never run looks identical to one that is merely
    queued. The recipes are the only place that relation is recorded, and not
    everyone has access to them.
    """
    files = glob.glob(os.path.join(recipes_dir, "**", "*.cfg"), recursive=True)
    if not files:
        print(
            f"WARNING: no recipes found under {recipes_dir}; "
            "dependency analysis skipped",
            file=sys.stderr,
        )
        return None

    files_of: dict = collections.defaultdict(set)
    deps: dict = {}
    for path in files:
        parser = configparser.ConfigParser(strict=False, interpolation=None)
        try:
            parser.read(path)
        except configparser.Error:
            continue
        if not parser.has_section("Main"):
            continue

        main = parser["Main"]
        name = (main.get("display_name") or main.get("name") or "").strip()
        if not name:
            continue

        # Dependency paths in a recipe are relative to the checkout root
        key = os.path.relpath(path, recipes_dir)
        files_of[name].add(key)
        deps[key] = (
            [
                value.strip()
                for option, value in parser["PullRequest Dependencies"].items()
                if option.startswith("filename")
            ]
            if parser.has_section("PullRequest Dependencies")
            else []
        )

    return {"files_of": dict(files_of), "deps": deps}


def downstream_of(graph: dict, contexts: Sequence[str]) -> set:
    """
    Get the display names that depend, directly or transitively, on the given ones.

    A display name can appear in more than one branch of the recipes, so a name
    counts as downstream if any of its recipe files is. That errs toward
    reporting a job as blocked rather than missing one.
    """
    dependents: dict = collections.defaultdict(set)
    for path, requires in graph["deps"].items():
        for required in requires:
            dependents[required].add(path)

    seen = set()
    frontier = {f for name in contexts for f in graph["files_of"].get(name, ())}
    while frontier:
        following = set()
        for path in frontier:
            for dependent in dependents.get(path, ()):
                if dependent not in seen:
                    seen.add(dependent)
                    following.add(dependent)
        frontier = following

    return {name for name, paths in graph["files_of"].items() if paths & seen}


def blocked_pending(graph: Optional[dict], statuses: Sequence[dict]) -> List[str]:
    """Get the pending jobs that are downstream of a failure, if recipes allow."""
    if not graph:
        return []
    failed = [s.get("context") for s in real_failures(statuses)]
    blocked = downstream_of(graph, [c for c in failed if c])
    return sorted(
        {
            s.get("context")
            for s in pending_jobs(statuses)
            if s.get("context") in blocked
        }
    )


def pending_jobs(statuses: Sequence[dict]) -> List[dict]:
    """
    Select the jobs that have not finished.

    Completeness is judged from the statuses that have been posted, so a job
    CIVET has not reported on at all is invisible here.
    """
    return [s for s in statuses if s.get("state") == PENDING_STATE]


def real_failures(statuses: Sequence[dict]) -> List[dict]:
    """
    Select the jobs that actually failed.

    Jobs CIVET skipped because a dependency failed also report a failure
    state, but there is nothing in them to fix.
    """
    return [
        s
        for s in statuses
        if s.get("state") in ("failure", "error")
        and s.get("description") != BLOCKED_DESCRIPTION
    ]


def print_digest(info: dict, state: dict, graph: Optional[dict] = None) -> None:
    """Print the job-level report."""
    statuses = state["statuses"]
    failed_jobs = real_failures(statuses)
    blocked = [s for s in statuses if s.get("description") == BLOCKED_DESCRIPTION]

    pending = pending_jobs(statuses)
    passed = [s for s in statuses if s.get("state") == "success"]

    counts = [f"{len(failed_jobs)} failed"]
    if blocked:
        counts.append(f"{len(blocked)} blocked")
    if pending:
        counts.append(f"{len(pending)} pending")
    counts.append(f"{len(passed)} passed")
    print(
        f"PR #{info['number']} {info['headRefName']} @ {info['headRefOid'][:7]}  "
        f"jobs: {', '.join(counts)} / {len(statuses)}"
    )

    if pending:
        # Every conclusion below is provisional while jobs are still running,
        # and GitHub's own rollup state does not reveal this
        print(
            f"\nEVENT INCOMPLETE: {len(pending)} job(s) still running. "
            "Failures seen so far are not the whole picture."
        )

    doomed = blocked_pending(graph, statuses)
    if doomed:
        print(
            f"\n{len(doomed)} of the {len(pending)} pending job(s) depend on a "
            "failed job and will not run:"
        )
        for name in doomed[:MAX_JOBS]:
            print(f"  {name}")
        if len(doomed) > MAX_JOBS:
            print(f"  ... and {len(doomed) - MAX_JOBS} more")

    if failed_jobs:
        print("\nfailed jobs:")
        for status in failed_jobs[:MAX_JOBS]:
            print(f"  {status.get('context')}  {status.get('target_url')}")
        if len(failed_jobs) > MAX_JOBS:
            print(f"  ... and {len(failed_jobs) - MAX_JOBS} more")

    if blocked:
        # Cascade effects of the failures above, with nothing of their own to fix
        print(
            "\nblocked by failed dependencies: "
            + ", ".join(s.get("context") or "?" for s in blocked)
        )


def json_payload(
    info: dict,
    state: dict,
    collected: dict,
    max_failures: int,
    graph: Optional[dict] = None,
) -> dict:
    """
    Build the machine-readable form of the same report.

    Capped the same way as the text output, with exact totals alongside, so
    that a consumer sees a bounded payload and still knows what was left out.
    """
    return {
        "pr": info["number"],
        "branch": info["headRefName"],
        "head_sha": info["headRefOid"],
        # GitHub's rollup, which reads "failure" while jobs are still pending
        "github_state": state["state"],
        "complete": bool(state["statuses"]) and not pending_jobs(state["statuses"]),
        "counts": {
            "jobs": len(state["statuses"]),
            "failed": len(real_failures(state["statuses"])),
            "blocked": sum(
                1
                for s in state["statuses"]
                if s.get("description") == BLOCKED_DESCRIPTION
            ),
            "pending": len(pending_jobs(state["statuses"])),
            "pending_blocked": len(blocked_pending(graph, state["statuses"])),
            "passed": sum(1 for s in state["statuses"] if s.get("state") == "success"),
        },
        # Passing jobs are omitted; they are counted above and carry nothing
        # a consumer would act on
        "jobs": [
            {
                "context": s.get("context"),
                "state": s.get("state"),
                "description": s.get("description"),
                "url": s.get("target_url"),
                "blocked": s.get("description") == BLOCKED_DESCRIPTION,
            }
            for s in state["statuses"]
            if s.get("state") != "success"
        ],
        "failed_jobs": collected["jobs"],
        "tests_total": len(collected["rollup"]),
        "tests": rollup_entries(collected["rollup"], max_failures),
        "hints": collected["hints"],
        "jobs_not_read": collected["jobs_not_read"],
        # Empty unless a recipes checkout was given with --recipes
        "pending_blocked": blocked_pending(graph, state["statuses"]),
    }


def parse_args(argv: Sequence[str]) -> argparse.Namespace:
    """Parse command-line arguments."""
    parser = argparse.ArgumentParser(
        description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter
    )
    parser.add_argument("--pr", type=int, help="PR number (default: branch's PR)")
    parser.add_argument(
        "--repo",
        type=str,
        help="owner/name of the repository (default: the upstream remote's)",
    )
    parser.add_argument(
        "--upstream-remote",
        type=str,
        help="Remote naming the upstream repo. Defaults to CIVET_UPSTREAM_REMOTE, "
        f"else the first of {', '.join(UPSTREAM_REMOTE_CANDIDATES)} that exists",
    )
    parser.add_argument(
        "--max-failures",
        type=int,
        default=DEFAULT_MAX_FAILURES,
        help=f"Max failures to describe (default: {DEFAULT_MAX_FAILURES})",
    )
    parser.add_argument(
        "--json",
        action="store_true",
        dest="as_json",
        help="Emit the report as JSON; implies --errors",
    )
    parser.add_argument(
        "--recipes",
        type=str,
        metavar="DIR",
        help="Optional civet_recipes checkout, used to report which pending "
        "jobs depend on a failed job and so will never run",
    )
    parser.add_argument(
        "--errors",
        action="store_true",
        help="Pull the logs of each failed job and extract build/test errors",
    )
    parser.add_argument(
        "--max-log-jobs",
        type=int,
        default=DEFAULT_MAX_LOG_JOBS,
        help=f"With --errors, max jobs to read (default: {DEFAULT_MAX_LOG_JOBS})",
    )
    parser.add_argument(
        "--max-diagnostics",
        type=int,
        default=DEFAULT_MAX_DIAGNOSTICS,
        help=f"With --errors, unique errors per step (default: {DEFAULT_MAX_DIAGNOSTICS})",
    )
    parser.add_argument(
        "--max-signatures",
        type=int,
        default=DEFAULT_MAX_SIGNATURES,
        help=f"With --errors, error signatures per step (default: {DEFAULT_MAX_SIGNATURES})",
    )
    parser.add_argument(
        "--job-log",
        type=str,
        metavar="JOB_URL",
        help="List the step logs of a CIVET job instead of reporting failures",
    )
    parser.add_argument(
        "--step",
        type=str,
        help="With --job-log, print a tail of the step whose name contains this",
    )
    parser.add_argument(
        "--lines",
        type=int,
        default=100,
        help="With --step, how many trailing lines to print (default: 100)",
    )
    return parser.parse_args(argv)


def main(argv: Sequence[str]) -> int:
    """Perform the main action; run from __main__."""
    args = parse_args(argv)

    if args.job_log:
        print_job_log(args.job_log, args.step, args.lines)
        return 0

    slug = resolve_repo(args.repo, args.upstream_remote)
    if not slug:
        tried = args.upstream_remote or os.environ.get("CIVET_UPSTREAM_REMOTE")
        tried = [tried] if tried else list(UPSTREAM_REMOTE_CANDIDATES)
        raise GitHubError(
            f"No repository found from remote(s) {', '.join(tried)}; "
            "pass --repo owner/name or set --upstream-remote"
        )

    info = resolve_pr(slug, args.pr)
    sha = info["headRefOid"]

    state = fetch_state(slug, sha)

    # Optional: without it, a doomed pending job is indistinguishable from a
    # queued one, which is a limit of the status data rather than a bug
    graph = load_recipe_graph(args.recipes) if args.recipes else None

    # JSON is for programmatic use, where the per-test detail is the point
    collect = args.errors or args.as_json
    collected = (
        collect_errors(
            real_failures(state["statuses"]),
            args.max_log_jobs,
            args.max_diagnostics,
            args.max_signatures,
        )
        if collect
        else None
    )

    if args.as_json:
        print(
            json.dumps(
                json_payload(info, state, collected, args.max_failures, graph),
                indent=1,
                sort_keys=True,
            )
        )
        return 0

    print_digest(info, state, graph)
    if collected:
        print_errors(collected, args.max_failures)

    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
