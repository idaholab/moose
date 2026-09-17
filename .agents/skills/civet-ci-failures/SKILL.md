---
name: civet-ci-failures
description: >-
  Use when investigating why CIVET CI failed on a pull request for MOOSE or a downstream app:
  which jobs failed, which tests inside them, why, and how to reproduce a failure locally. Reads
  GitHub commit statuses and CIVET job logs through the gh CLI, so it works without CIVET
  credentials. Also covers how to read the result, because raw job counts and GitHub's own
  rollup state are both misleading.
---

# CIVET CI failures on a pull request

## The tool

`scripts/civet_pr_failures.py` in the MOOSE repo. It needs an authenticated `gh` CLI and network
access to `civet.inl.gov`; nothing else. It combines two sources:

- **GitHub commit statuses**, which CIVET posts one per job, giving job state and the CIVET job URL.
- **CIVET job step logs**, served as a gzipped tarball per job with no authentication for public
  recipes, which is where per-test failures and build errors actually live.

```bash
scripts/civet_pr_failures.py                        # the current branch's PR, job level only
scripts/civet_pr_failures.py --pr 33645             # a specific PR
scripts/civet_pr_failures.py --pr 33645 --errors    # also read logs: build errors, failing tests
scripts/civet_pr_failures.py --pr 33645 --errors --recipes ~/projects/civet_recipes
scripts/civet_pr_failures.py --pr 33645 --json      # same findings as JSON; implies --errors
scripts/civet_pr_failures.py --sha <commit>         # a commit rather than a PR
```

CIVET attaches its statuses to the commit, not to the pull request, so `--sha` is enough on its own
and covers push events to `next` and `devel`, where there is no pull request to name.

The repository is taken from `--repo owner/name`, else the remote named by `--upstream-remote` or
`$CIVET_UPSTREAM_REMOTE`, else the first of `up`, `upstream`, `origin` that exists. A developer
checkout often has `origin` on a personal fork and many remotes for reviewing others' branches, so
never rely on `gh`'s own default repository resolution.

`--recipes` is optional and points at a `civet_recipes` checkout. Without it, a pending job that
can never run is indistinguishable from one that is merely queued.

To read a log, use `--job-log <civet job url>` for a listing of steps with sizes, then
`--job-log <url> --step <name> --lines N` for a bounded tail. Never pipe a whole job tarball into
context; a single step can be several megabytes.

## Reading the result correctly

Four traps, each of which has produced a confidently wrong conclusion:

**A "failed" job is often a cascade, not a cause.** CIVET marks jobs it skipped because a
dependency failed with the description `Won't run due to failed dependencies`, and GitHub reports
those as failures. One real failure can present as dozens. The report separates them; count only
the real ones.

**GitHub's combined `state` is not a completion signal.** It turns `failure` the moment any single
job fails, while every other job is still pending. Never treat `state != "pending"` as "the event
finished."

**Absence of pending statuses is not completion either.** A job that has not started has no status
at all, so an event that is ramping up momentarily looks finished — particularly right after an
invalidation. Require the counts to hold still across two polls before believing it.

**A "passed" job may have failed.** CIVET maps its `FAILED_OK` status to GitHub `success` with the
description `Failed but allowed`. Check descriptions before declaring a job clean.

## Read the reason, not the status

A TestHarness failure line carries a status word and a reason: `ERROR <test> FAILED (KILLED: OVER
MEMORY)`. The status word is `ERROR`; the actionable part is the reason. Reporting the status alone
hides memory kills and timeouts behind a generic label.

Cluster before you report. A single defect surfaces in many tests: one Kokkos error
(`Retrieving a Kokkos function as abstract type is currently not supported for GPU`) accounted for
113 failing tests across five test steps. The number of distinct error signatures is what says how
many things went wrong; the number of failing tests does not.

## Three categories of failure, not two

- **Caused by the PR.** The default assumption for MOOSE: the base branch is rarely red.
- **Spurious / infrastructural.** A network failure fetching a dependency during a container build
  (`curl: (28) ... vtk.org`), or a recipe copying tests to a temp directory without their mesh
  files (`Error opening ExodusII mesh file: /tmp/...`). These need a re-run, not a code change.
- **Genuinely pre-existing.** Rare. Do not conflate it with spurious; a flaky failure is not a
  base-branch failure.

Build failures only ever appear in the logs. The results database never sees them, because the job
dies before results are stored — so `--errors` is the only route to them.

## Known remediations by reason

The report attaches these to the test they apply to. Correct them as they prove wrong.

| Reason | What it means and what to do |
| --- | --- |
| `KILLED: OVER MEMORY` | The harness enforced `--max-memory-per-slot`. Raise `min_slots` in the spec. Reproducible on Linux, where memory accounting is consistent; not on macOS. |
| `TIMEOUT` | Often a slow build rather than the test: `-O0` coverage jobs and `METHOD=dbg` jobs are both far slower than opt. If a test times out only there, reduce its problem size rather than raising `max_time`. |
| `EXPECTED OUTPUT MISSING` | `RunApp`'s `expect_out` pattern did not match. Update `expect_out`, or fix the output it describes. |
| `EXPECTED OUTPUT NOT FOUND` | Comes only from `PetscJacobianTester` and `TaoGradientTester`, which grep for a specific diagnostic line. The run usually died before PETSc/TAO printed it — look for the real error earlier in the same output. Nothing to do with gold files. |
| `EXPECTED ERROR MISSING` | The error text a `RunException` test expects has changed. Update `expect_err`. |
| `curl: (N)` / `FATAL:` in a container build | A download or the container build failed. Spurious; re-run. |
| `fatal: unable to access` / `Empty reply from server` | A git clone or fetch failed during a build. Spurious; re-run. |
| `fatal: remote error` | The ref being fetched is not on the remote, usually a submodule bumped to a commit that was never pushed upstream. Push it, then re-run. |

Which cause the report shows is decided by precedence: a real compiler or linker diagnostic wins
outright, then a failure of the surrounding machinery, then the build system's own complaint. Lines
like `ninja: error: loading 'build.ninja'` and `fatal: unable to access` match the diagnostic
pattern but only report a failure they inherited, so they are demoted — otherwise a container build
reports the symptom and buries the download or clone that actually failed.

An `expect_out` that pins a floating-point value to an exact literal will fail once the run
changes process count or mesh distribution, because summation order does. Match the claim and let
the code's own tolerance enforce the value, as the sibling checks in the same spec do.

## Reproducing a failure

The report's reproduce command is extracted from the invocation the step actually logged, not
synthesized from the step name. That matters: real invocations carry flags like
`--compute-device=cuda`, `--min-parallel 7` and `--only-tests-that-require=mfem` that cannot be
guessed. When a test failed in several modes, the simplest failing invocation is reported, and the
mode count says whether the failure is specific to one way of running.

It also reports the container the step ran in, read from the step header. Containers are recorded
per step rather than per job — a fetch step commonly runs in a base image while the steps that build
and test run in a versioned one — so the container shown is the one belonging to the invocation
shown. Reproducing the command outside it can behave differently, which is why the two are reported
together.

## Watching an event to completion

The tool reports a snapshot and deliberately does not block. A MOOSE event runs for hours, so watch
it in the background rather than waiting inside a turn.

A poll loop has to get three things right, none of which GitHub hands you:

```bash
# Re-resolve the head each cycle, so a new push is not polled against a stale SHA
SHA=$(gh pr view <N> --repo <owner/name> --json headRefOid --jq .headRefOid)

# Newest status per context wins: CIVET posts one when a job starts and another when it finishes
L=$(gh api "repos/<owner/name>/commits/$SHA/statuses" --paginate --jq '.[]' \
    | jq -sc 'group_by(.context) | map(max_by(.created_at))')

# Count real failures, excluding dependency cascades
echo "$L" | jq '[.[]|select((.state=="failure" or .state=="error")
     and (((.description//"")|contains("failed dependencies"))|not))]|length'
```

Treat the event as finished only when nothing is pending **and** the counts are unchanged from the
previous poll. Poll every few minutes, not every few seconds.

**Pushing to a PR cancels the in-flight event.** Every incomplete job is marked canceled and the
build clients are told to stop, so a push discards all progress on the current event and restarts
the matrix from zero. Recorded step output survives (only invalidation deletes that), but the
remaining work does not. So fix continuously and push once, in a batch.

The exception: when a failure is early and broad — a `Precheck` failure, or a framework compile
error that dooms most jobs — the rest of the event has nothing left to tell you. Push immediately
rather than waiting.

## Limits

- Private ("controlled apps") recipes return HTTP 403 for their logs. Those failures are invisible
  to this tool and need the CIVET web UI.
- Completeness is judged from statuses that have been posted, so a job CIVET has not reported on at
  all is invisible.
- `--recipes` matches jobs to recipes by display name, which can appear in more than one branch of
  the recipes tree, so it errs toward reporting a job as blocked.

## When something does not fit

Ask the user rather than guessing. This tool reports what CIVET recorded, and several things it
cannot settle on its own:

- A failure whose cause is not in the logs, or one whose reason is not in the table above.
- A remediation here that contradicts what you observe. The table is a starting point, not a
  specification.
- A job whose logs return 403. The user can open it in the CIVET web UI; you cannot.
- Whether a failure is worth fixing at all. A spurious failure needs a re-run, and only the user
  can decide to invalidate an event or re-push.

Say what you found, what you could not determine, and what you would do next. Do not invent a cause
for a failure you cannot read, and do not apply a remedy on a hunch.

## Confidence in what is written here

This skill was written from one extended investigation of a single pull request. Parts of it are
verified and parts are hypothesis; the difference matters if something here contradicts what you
observe.

Verified against source, or stated directly by a MOOSE developer:

- The `fatal: unable to access` row and the diagnostic precedence above, both checked against a
  container build that failed cloning a dependency.
- Reporting on a commit with `--sha`, and the per-step container extraction, both checked against a
  push event to `next`.
- The three failure categories, and that MOOSE's base branch is rarely red.
- `min_slots` as the remedy for `KILLED: OVER MEMORY`.
- `EXPECTED OUTPUT MISSING` originating in `RunApp`'s `expect_out`, and `EXPECTED OUTPUT NOT FOUND`
  coming only from `PetscJacobianTester` and `TaoGradientTester` — both read in
  `python/TestHarness/testers/`.
- That pushing to a PR cancels the in-flight event and stops its running jobs, read in the CIVET
  server and client source.
- The four reading traps. Each one produced a confidently wrong conclusion before being caught.

Inferred, not verified. Treat as a starting hypothesis and correct it in place:

- `TIMEOUT` as a slow-build artifact rather than a slow test. Drawn from which jobs one PR's
  timeouts landed in (`-O0` coverage and `METHOD=dbg` builds), not from timing measurements.
- `EXPECTED ERROR MISSING` mapping to `expect_err`. Symmetric with the `expect_out` case, but not
  read in the tester source.
- The `curl:` / `FATAL:` container-build row, generalized from a single upstream outage.
- The `fatal: remote error` row. The remedy was confirmed once in practice — pushing the submodule
  commit cleared the failure — but the pattern itself was never matched against real output,
  because the job carrying it was re-run before it could be tested.

Not exercised at all:

- The reproduce commands are extracted from the invocation each step logged, and were never run
  locally to confirm they reproduce the failure.
- `scripts/civet_pr_failures.py` has no unit tests, unlike the rest of `python/TestHarness`. Its
  report has been checked by hand against real events; its behaviour on an event shape not seen
  during that investigation is unknown.
