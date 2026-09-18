# CIVET Failure Report

`civet_ci_failures.py` reports why [CIVET](https://civet.inl.gov) failed for a pull request,
a commit, or a single job, in a compact, bounded form. It reads GitHub commit statuses and
CIVET job step logs through the `gh` CLI, so it needs no CIVET credentials.

## Standard Usage

There are three ways to name what to report on: a pull request, a commit (which is what
makes push events on `next` and `devel` reportable), or a single CIVET job named by its URL:

```text
python/civet_ci_failures/civet_ci_failures.py --pr 33645
python/civet_ci_failures/civet_ci_failures.py --sha <commit>
python/civet_ci_failures/civet_ci_failures.py --job <civet job url>
```

Without `--pr`, `--sha`, or `--job`, it looks up the current branch's open pull request by
name and reports on that; before the branch has an open pull request, it exits with an error
naming the branch, since CIVET itself has nothing to report on a branch until a pull request
puts a commit in front of it (a push to `next` or `devel` is the one exception, reachable
with `--sha`). Add `--errors` to also read each failed job's logs for build errors and
failing tests, or `--json` for the same findings as JSON (which implies `--errors`).

`-h` lists every option, including `--recipes` (a `civet_recipes` checkout, used to tell a
pending job that can never run apart from one that is merely queued), and the log-reading
options `--list-steps`, `--step`, `--grep`, `--context`, and `--lines` used with `--job`.

## Behavior

The output is deliberately small: passing jobs are counted rather than listed, and every list
of failures or diagnostics is capped (`--max-failures`, `--max-log-jobs`, `--max-diagnostics`,
`--max-signatures`). See the `civet-ci-failures` skill for how to read the report: which
counts are misleading, which reasons carry known remediations, and how to reproduce a failure
locally.
