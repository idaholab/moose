---
name: civet-ci-failures
description: >-
  Use when investigating why CIVET failed for MOOSE or a downstream app, whether the starting
  point is a pull request, a commit, or a single CIVET job URL: which jobs failed, which tests
  inside them, why, and how to reproduce a failure locally. Reads GitHub commit statuses and CIVET
  job logs through the gh CLI, so it works without CIVET credentials. Also covers how to read the
  result, because raw job counts and GitHub's own rollup state are both misleading.
---

# CIVET failures

## The tool

`python/civet_pr_failures/civet_pr_failures.py` in the MOOSE repo. It needs an authenticated `gh` CLI and network
access to `civet.inl.gov`; nothing else. It combines two sources:

- **GitHub commit statuses**, which CIVET posts one per job, giving job state and the CIVET job URL.
- **CIVET job step logs**, served as a gzipped tarball per job with no authentication for public
  recipes, which is where per-test failures and build errors actually live.

```bash
python/civet_pr_failures/civet_pr_failures.py                        # the current branch's PR, job level only
python/civet_pr_failures/civet_pr_failures.py --pr 33645             # a specific PR
python/civet_pr_failures/civet_pr_failures.py --pr 33645 --errors    # also read logs: build errors, failing tests
python/civet_pr_failures/civet_pr_failures.py --pr 33645 --errors --recipes ~/projects/civet_recipes
python/civet_pr_failures/civet_pr_failures.py --pr 33645 --json      # same findings as JSON; implies --errors
python/civet_pr_failures/civet_pr_failures.py --sha <commit>         # a commit rather than a PR
python/civet_pr_failures/civet_pr_failures.py --job <civet job url>  # a single job, named by its URL
```

CIVET attaches its statuses to the commit, not to the pull request, so `--sha` is enough on its own
and covers push events to `next` and `devel`, where there is no pull request to name.

The repository is taken from `--repo owner/name`, else the remote named by `--upstream-remote` or
`$CIVET_UPSTREAM_REMOTE`, else the first of `up`, `upstream`, `origin` that exists. A developer
checkout often has `origin` on a personal fork and many remotes for reviewing others' branches, so
never rely on `gh`'s own default repository resolution.

`--recipes` is optional and points at a `civet_recipes` checkout. Without it, a pending job that
can never run is indistinguishable from one that is merely queued.

## Start from a job URL when that is what you were given

A job URL is a complete starting point on its own. `--job` reads that job's logs and reports the
same per-test findings `--errors` gives, plus what the URL does not carry: the recipe, the event,
the pull request, and both commits.

```bash
python/civet_pr_failures/civet_pr_failures.py --job https://civet.inl.gov/job/4180979/
```

```
Parallel sweep odds (weekly extra)  https://civet.inl.gov/job/4180979/
  pull request 32394 (Pull request alternatives)
  head lindsayad/moose:fix-ptscotch-test @ 853397b7b989
  base idaholab/moose:devel @ 92670c42317d
  04_Test_-p_13 (exit 128, 1 tests failed, 1 error signature(s), no retry attempted)
    x1  The client timed out during initialization

unique failing tests: 1
  controls/web_server_control.connect_port  [EXIT CODE 1 != 0]
    cd test && ./run_tests -p 13 --re '^controls/web_server_control\.connect_port$'
```

Read the header before the failure. The recipe name says what the job was running, which is context
the failure line does not carry: a sweep of odd process counts, or a weekly extra, exercises a mode
the rest of the matrix does not. Treat that as something to suspect and then confirm. It does not
establish that the failure is specific to that mode, because another recipe may be running a
different mode that fails the same test.

Confirming it takes the whole event, which one job cannot give you. Pass the pull request number
from the header to `--pr` to see which other jobs failed the same test and in which modes, and
whether the branch is implicated at all.

## Reading a raw log

Only for what the report does not cover. Start with `--list-steps` for sizes, then read one step:

```bash
python/civet_pr_failures/civet_pr_failures.py --job <url> --list-steps
python/civet_pr_failures/civet_pr_failures.py --job <url> --step 04_Test --grep 'Failed to bind' --context 5
python/civet_pr_failures/civet_pr_failures.py --job <url> --step 01_Build --lines 200
```

Use `--grep`, not `--lines`, for a test failure. The TestHarness prints each failure where it
happens and then runs thousands more tests, so the detail sits in the middle of the log and no
affordable tail reaches it. `--lines` bounds both modes and defaults to 100, and grep output marks
elided regions with `...`, so a printed block is contiguous only where it says it is.

Never pipe a whole job tarball into context; a single step can be several megabytes.

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

The signature is the message the application printed, which is not always the cause. A MOOSE error
block reports the failure the run died of, and a failure inside a thread or a subprocess started
earlier can be printed before that block and then subsumed by it. When a signature reads as a
timeout, a missing connection, or anything else that is plainly a consequence, `--grep` the same
step for what came before it rather than treating the signature as the diagnosis.

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
dies before results are stored — so `--errors` (or `--job`, which always reads them) is the only
route to them.

## Known remediations by reason

The report attaches these to the test they apply to. Correct them as they prove wrong. Rows marked
*(hypothesis)* were reasoned out rather than confirmed against a real failure, so check one before
you act on it and rewrite the row with what you find.

| Reason | What it means and what to do |
| --- | --- |
| `KILLED: OVER MEMORY` | The harness enforced `--max-memory-per-slot`. Raise `min_slots` in the spec. Reproducible on Linux, where memory accounting is consistent; not on macOS. |
| `TIMEOUT` | Often a slow build rather than the test: `-O0` coverage jobs and `METHOD=dbg` jobs are both far slower than opt. If a test times out only there, reduce its problem size rather than raising `max_time`. |
| `EXPECTED OUTPUT MISSING` | `RunApp`'s `expect_out` pattern did not match. Update `expect_out`, or fix the output it describes. |
| `EXPECTED OUTPUT NOT FOUND` | Comes only from `PetscJacobianTester` and `TaoGradientTester`, which grep for a specific diagnostic line. The run usually died before PETSc/TAO printed it — look for the real error earlier in the same output. Nothing to do with gold files. |
| `EXPECTED ERROR MISSING` | The error text a `RunException` test expects has changed. Update `expect_err`. |
| `curl: (N)` / `FATAL:` in a container build | *(hypothesis)* A download or the container build failed. Spurious; re-run. Generalized from a single upstream outage. |
| `fatal: unable to access` / `Empty reply from server` / `Could not resolve host` | A git clone or fetch failed during a build, so the remote was never reached. Spurious as far as the branch is concerned. Check which host the failing URL names: submodules hosted outside `github.com` and `github.inl.gov` are outside MOOSE's control, and when one of those hosts is down a re-run only succeeds once it recovers. |
| `fatal: remote error` | *(hypothesis)* The ref being fetched is not on the remote, usually a submodule bumped to a commit that was never pushed upstream. Push it, then re-run. The remedy was confirmed once in practice; the pattern itself has never been matched against real output. |

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

These commands may not have been run locally to confirm they reproduce the failure they came from.
Offer one as the invocation the step logged, not as a command known to fail.

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
- `--job` reads one job in isolation, so it cannot tell a cascade from a cause, or an isolated
  failure from a broad one. Follow it with `--pr` on the number it reports.
- The recipe name, pull request and commits `--job` reports come from the CIVET environment each
  step dumps in its header. A job whose logs are unreadable, private, or truncated before that dump
  reports them as `?`. `--job` has only been run against a pull request job; a push event to `next`
  has no `CIVET_PR_NUM`, and that path is untested.
- Unit tests cover the parsing, clustering, and reproduce-command logic directly; the `gh` and
  network calls are only exercised against mocks, never a live CIVET event. A change to CIVET's
  own output shape, or a log format this tool has not seen, can still slip past the test suite.

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

`python/doc/content/python/civet_pr_failures.md` is the user-facing page for the tool, and
`python/civet_pr_failures/tests` holds its unit tests. A change to the script belongs with an
update to both.
