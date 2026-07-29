---
name: moose-verify-changes
description: >-
  Select and run focused builds, tests, and executable checks for MOOSE
  changes, with a mandatory environment gate, protection against stale
  binaries, and evidence-based results. Use when someone asks to build, test,
  or verify MOOSE work; before claiming that changed MOOSE behavior works; or
  when another MOOSE workflow needs verification for a pull request, review
  response, bug fix, or feature.
---

# Verifying MOOSE Changes

Use the smallest set of checks that establishes whether the changed behavior
works. This skill owns verification execution and evidence; it does not decide
whether a PR is ready, assess test SQA metadata, modify review conversations,
or publish anything.

## Step 1 - Establish the verification scope

1. Find the repository root and read the applicable `AGENTS.md` files for the
   changed paths, bounded by the repository root.
2. Identify the changed behavior, affected targets, and the tests or
   executables that directly exercise it. Use repository test manifests and
   nearby tests; use CodeGraph when available if callers or impact are unclear.
3. Map each changed behavior to at least one planned check. Prefer focused
   checks, expanding to broader suites only when the blast radius or user
   request warrants it.
4. State the planned build and test commands, working directories, and what
   each check establishes.

Verification does not authorize edits to source, tests, gold files, or
configuration, and it never authorizes Git operations.

## Step 2 - Pass the environment gate

Before building or performing verification, including running `run_tests` or
invoking a pre-existing MOOSE executable, ask whether the user's MOOSE stack
uses conda unless the conversation already establishes it. If it does, ask
which conda environment to activate and wait for the answer. Activate that
environment for every verification command.

Do not inspect or run an existing binary, rely on the current shell state, or
use an earlier result as a shortcut around this gate. If the user declines or
cannot provide a required environment, mark the affected checks as not run and
return that limitation to the calling workflow. Never imply that they passed.

## Step 3 - Build the code under test

When changed source must be compiled, build the required target in the selected
environment before running it. The presence or timestamp of a pre-existing
binary is not proof that it contains the current changes. It is acceptable for
the build system, after the environment gate, to establish that the target is
up to date.

Do not broaden the build merely for completeness. If the necessary target
cannot be identified or built, report the verification as blocked instead of
substituting a different binary.

## Step 4 - Run and interpret the checks

Run the planned focused checks and record:

- The exact command and working directory.
- The source revision or working-tree state tested.
- Pass, fail, blocked, or not-run status.
- The relevant output and what behavior the result covers.

Do not hide a failure through repeated retries, narrower filters, relaxed
tolerances, or updated expected output. Diagnose enough to distinguish a code
failure from an environment or infrastructure problem when the evidence
supports that distinction. Any proposed repair is a separate change and
requires the authorization appropriate to the calling task.

## Step 5 - Return verification evidence

Return a concise check-by-check result with failures and unrun checks first.
State what remains unverified and avoid a blanket success claim when the
focused checks cover only part of the change.

Leave PR-readiness decisions, review-thread state, and publication decisions to
the workflow that invoked this skill.
