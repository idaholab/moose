---
name: moose-pr-review
description: >-
  Review a MOOSE pull request or local branch against MOOSE's contribution standards:
  applicable AGENTS.md guidance, the MOOSE Code Standard (SCS), the SQA testing rules
  (every test is a requirement, with requirement/design/issues), required documentation
  stub pages for new objects, and PR governance (issue references, scope,
  required-vs-suggested phrasing). Use this whenever someone wants to review MOOSE changes
  or asks "review this PR", "review my branch", "is this ready to merge", "check my changes
  before I submit", or wants pre-submission feedback on code in the MOOSE framework or its
  modules - even if they never say the word "review". This is the MOOSE-aware review layer;
  it complements the generic /code-review and /security-review skills and defers to them
  for deep correctness and security analysis rather than duplicating that work.
---

# Reviewing a MOOSE Pull Request

MOOSE is safety-relevant software with a strict software-quality program (NQA-1). A review
is not just "does the code look right" - it is the gate that enforces the Code Standard, the
SQA traceability between tests and requirements, and the documentation that lets the next
person understand the change without asking the author. This skill applies the same bar a
Change Control Board reviewer would, expressed as concrete, checkable steps.

This skill is the MOOSE-specific layer. It does **not** re-implement generic bug hunting or
security analysis - those are covered well by the built-in `/code-review` and
`/security-review` skills. Run or recommend those for deep correctness/security; spend this
skill's attention on what is unique to MOOSE and what CI cannot auto-check.

Use `moose-verify-changes` for any build or test execution. This skill assesses
whether the change has appropriate tests and SQA traceability; it does not
duplicate the verification workflow.

## What CI already enforces (do not nitpick by hand)

Formatting is checked automatically on every PR, so manual review time should not be spent on
it. If you see violations, note them once and point to the fix command rather than listing
each instance:

- C++ formatting: `git clang-format <base>` (uses `.clang-format`)
- Python formatting: `black .` (uses `pyproject.toml`)
- No trailing whitespace / tabs, and the "at least one development-branch commit
  references an issue" check

Focus human/AI review on the semantic standards CI can't see: naming, const-correctness,
access control, API design, C-vs-C++ construct choices, tests, and documentation.

## Step 0 - Preflight: CodeGraph is required (do not skip)

This review depends on CodeGraph to detect code that reinvents functionality MOOSE already has -
the single most valuable thing this review catches against MOOSE's "use existing functionality,
don't reimplement it" rule (AGENTS.md, Simplicity First), and something grep cannot do reliably
across a 100k-symbol codebase. For review, CodeGraph is a hard prerequisite, not an optimization.

Before anything else, confirm the index exists: check for a `.codegraph/` directory at the
repository root (or run `codegraph status`). If it is missing, **stop and surface it to the user -
do not fall back to grep and proceed** - and tell them how to set it up:

```bash
codegraph install   # one-time: register the CodeGraph MCP server with this agent
codegraph init      # build the index for this repo (run from the repo root; takes a few minutes)
```

Only continue once the index is present. For how to use CodeGraph - the query patterns, the
reuse-detection workflow, and blast-radius checks - use the `moose-codegraph` skill
(`.claude/skills/moose-codegraph/SKILL.md`). This review layer adds the hard requirement above
and the review-specific framing below rather than repeating those mechanics.

## Step 1 - Identify what you are reviewing

Determine the target and compute the diff. Two cases:

**A GitHub PR** (user gives a number or URL, or asks to review a specific PR):

```bash
gh pr view <num> --json title,body,baseRefName,baseRefOid,headRefName,author,files,commits,state,statusCheckRollup
gh pr diff <num>
```

`baseRefName` is the merge target (MOOSE PRs target `next`); `baseRefOid` identifies the exact
target revision whose review policy governs the PR. Use the PR body to read the author's stated
Reason / Design / Impact (the PR template fields).

**A local branch** (the default when no PR is named):

```bash
# Review the branch against the point it was cut from. <base> is the integration branch this
# work will merge into. For MOOSE the branch name is `next`, but do not assume a remote name:
# resolve the remote-tracking ref for the remote that represents the canonical MOOSE repository.
# This is NOT the branch's upstream tracking branch: `git rev-parse @{u}` resolves to this same
# branch's remote copy (e.g. <fork>/<branch>), not the base. Use three dots so the diff shows
# only what this branch added, regardless of how far the integration branch moved.
git diff <base>...HEAD              # the changes under review
git log <base>..HEAD --format='%h %s'   # the commits under review - read them for intent and scope
```

Then categorize the changed files - the review checklist depends on what kinds of files
changed:

- C++ (`*.h` / `*.C`) - apply the Code Standard and look for new registered objects
- Python (`*.py`) - Code Standard (Python section)
- Test specs (files literally named `tests`) and inputs (`*.i`) - SQA testing checks
- Markdown under `doc/content/` - documentation checks
- Build/config (`Makefile`, `*.mk`, `configure.ac`) - flag for extra scrutiny

## Step 2 - Gather MOOSE context

- **Applicable `AGENTS.md` guidance.** Read the `AGENTS.md` files from the merge target
  revision, not copies changed by the PR itself. For a GitHub PR this revision is `baseRefOid`;
  for a local branch it is `<base>`, resolved from the canonical MOOSE remote rather than an
  assumed remote name. List tracked guidance with
  `git ls-tree -r --name-only <base-revision> | rg '(^|/)AGENTS\.md$'` and read applicable
  files with line numbers using `git show <base-revision>:<path> | nl -ba` (fetch the base
  revision first if necessary). The root file applies repository-wide; a nested file applies
  only to its directory subtree; and the deepest applicable file overrides a parent only where
  they conflict. Map the resulting root-to-deep guidance chain to every changed path. If the PR
  adds or changes an `AGENTS.md`, review that file as a policy/documentation change, but do not
  let it retroactively govern the same PR.
- **Linked issue(s).** Read the issue(s) referenced by any commit in the development branch -
  they are your spec for judging whether the change is complete and correctly scoped. (CI already
  enforces that at least one commit contains a reference, so don't spend review time policing its
  presence or require every commit to repeat it.)
- **Newly registered objects.** Find new user-facing objects in the diff:
  `git diff <base>...HEAD | grep -E '^\+.*register(MooseObject|.*Action)\('`.
  Each new object drives both a documentation check (Step 3E) and a testing check (Step 3D).
- **Scope.** Is the change surgical and focused, or does it bundle unrelated edits ("rider"
  commits)? MOOSE reviewers may legitimately ask to split a sprawling PR. Note mixed concerns.

## Step 3 - Review across the MOOSE dimensions

Work top-down: assess the high-level design first (a sound design with rough edges is fixable;
a wrong design may not be), then the details. For each dimension, the highest-value checks are
summarized here; the cited files hold the full checklists - read the relevant one when the diff
touches that area.

**A. Design, scope, and user interface** (`framework/doc/content/framework/reviewing.md`)
- Is the high-level design sound, and proportionate scrutiny applied (changes affecting many
  users or that others will build on deserve more)?
- Scrutinize the **user interface**: new input parameters, their names, defaults, and whether
  `validParams` documentation strings make them self-explanatory. Interface mistakes are
  expensive because users depend on them.
- Will the code be understandable outside the PR context, from comments and docs alone?

**B. Reuse and duplication (CodeGraph-driven - vital)** -> use the `moose-codegraph` skill
- For every new function, class, or method the PR introduces, run the reuse-detection workflow
  from the `moose-codegraph` skill to ask whether MOOSE already provides it. Reinventing an
  existing utility, algorithm, or object is one of the most common and most important things a
  review catches - it bloats the codebase, diverges behavior, and increases maintenance costs.
  If a near-duplicate exists, flag it as **required** and point the author to reuse or extend it.
- Also run `codegraph_callers` / `impact` on changed existing symbols to judge blast radius (this
  feeds the scope/scrutiny assessment in A) and to find call sites the PR should update.

**C. MOOSE Code Standard** -> read `references/code-standard.md`
- Top semantic checks CI misses: strict **const-correctness** (logically-const-but-unmarked is
  a design issue), **access control** (default `private`; minimal `public`; ordered
  public->protected->private), C++-over-C constructs (`nullptr`, `static_cast`, `enum class`,
  RAII, `make_unique`/`make_shared`), header hygiene (forward-declare in headers, include in
  `.C`), `override`, virtual destructors, and the naming conventions.

**D. Tests and SQA** -> read `references/testing-sqa.md`
- Every new feature/behavior needs a test, and **every test is a requirement**. Each test
  block must carry an unambiguous `requirement`, a `design` (markdown path), and `issues`
  (`#numbers`). Error paths deserve `RunException` tests. Diff-based tests need gold files.
- Missing or under-specified tests are the most common required findings - check this carefully.

**E. Documentation** -> read `references/documentation.md`
- Each new MooseObject needs `addClassDescription(...)` in `validParams()` **and** a markdown
  stub page mirroring the source path (e.g. `src/kernels/Foo.C` ->
  `doc/content/source/kernels/Foo.md`). A missing stub page fails the docs build.
- Significant changes warrant a newsletter entry (current month, e.g.
  `modules/doc/content/newsletter/2026/2026_06.md`).

**F. Applicable `AGENTS.md` guidance (observable rules only)**
- Assess rules whose compliance can be established from the diff and necessary surrounding
  context. Typical examples cover simplicity, reuse, surgical scope, unrelated cleanup,
  newly orphaned code, comment preservation and clarity, exact code-style constructs, and
  tests required by the changed behavior.
- Silently skip process-only rules that the review artifacts cannot prove, such as whether the
  author stated assumptions before coding, presented a plan, chose a particular tool, or asked
  before activating a conda environment. Do not infer a violation and do not spend report space
  listing unverifiable rules.
- Report only violations introduced or caused by the change. Do not turn pre-existing code
  outside the diff into review findings.
- Treat direct mandates and prohibitions (`must`, `use`, `do not`, `never`) as required changes.
  Treat preferences and recommendations as suggestions unless another applicable MOOSE standard
  already makes the issue required.
- Cite both the changed `path:line` and the governing `<path>/AGENTS.md:line`. When an existing
  review dimension catches the same problem, emit one finding and cite both standards rather
  than duplicating it.

**G. Correctness and security** (defer, don't duplicate)
- For logic bugs, edge cases, and efficiency, run the built-in `/code-review` (or recommend it).
- For anything security-sensitive (parsing untrusted input, file/system access, memory safety),
  run or recommend `/security-review`. Memory-management-heavy code may warrant a valgrind
  testing recipe; loose solve tolerances often break parallel testing - mention these when relevant.

## Step 4 - Write the report

The central organizing principle of a MOOSE review is separating what the author **must**
change from what you **suggest**. Be explicit about which is which (per
`framework/doc/content/framework/reviewing.md`): required items use imperative phrasing
("Mark this parameter `const`"); suggestions use "I suggest" / "consider". Treat minor items
(typos, grammar, docstrings) as required. Every finding should be actionable. Anchor
line-specific findings to a `path:line`; for a cross-cutting scope finding, name the affected
files or commits.

Use this structure:

```
# MOOSE PR Review: <title or branch name>

## Summary
<2-4 sentences: what the change does, overall design assessment, and a recommendation
(approve / approve-with-nits / changes-required). Name the applicable AGENTS.md files and
state either "no observable violations" or the number of required changes and suggestions
arising from them. Do not mention skipped process-only rules.>

## Required changes
1. `path/to/file.C:42` - <what is wrong and why it must change> _(agents | standard | tests | docs | design)_
2. ...
(If none: "None.")

## Suggestions
1. `path/to/file.h:17` - I suggest <improvement and rationale>
2. ...

## Tests & SQA
<Coverage of new behavior; presence/quality of requirement/design/issues; gold files; error-path tests.>

## Documentation
<Stub pages for new objects; addClassDescription; param doc strings; newsletter entry if warranted.>

## Checklist
- [ ] Applicable AGENTS.md files resolved from the merge target for every changed path
- [ ] Observable AGENTS.md guidance checked without reporting process-only rules
- [ ] CodeGraph available and used to check for reinvented functionality
- [ ] No new symbol duplicates existing MOOSE functionality (or reuse justified)
- [ ] Code Standard (const-correctness, access control, naming, C++ constructs)
- [ ] Tests present with requirement/design/issues
- [ ] Docs: stub pages + class descriptions for new objects
- [ ] Ran/recommended /code-review and /security-review as warranted
```

Explain the *why* behind each finding briefly - a reviewer who only says "wrong" teaches
nothing; one who says "mark this `const` because it doesn't mutate observable state, which
clarifies the API and lets the compiler help" leaves the author better off.

## Step 5 - Optionally post to the PR

Posting is outward-facing and visible to the author and the community, so **never post without
explicit confirmation** from the user, even if they earlier asked for a review. Ask first -
prompt for the exact posting intent (see below) - show them the full draft (the review body *and*
every inline comment with its `file:line` anchor and any suggestion block), then post only what
they approve.

**Disclose that the review is AI-generated.** This skill enforces MOOSE's standards, so it must
honor them itself: MOOSE expects AI-authored contributions to be disclosed, and a review is no
different. Never post a review that reads as if a human wrote it. State that AI produced it and
name the model, using the model powering this session (e.g. "Claude Opus 4.8 via Claude Code") -
read the exact name from your environment rather than hardcoding it, since it changes over time.

- On the **review body**, end with a one-line footer, e.g.:
  `_Review generated by <model> via Claude Code, at the request of @<user>._`
- On **every inline comment**, append a short tail, e.g. ` _(generated by <model>)_`, so each
  comment is self-identifying even when read out of the review's context.

**Before posting, ask the user for the exact posting intent - do not infer it.** Posting is not a
single yes/no. Surface these as concrete choices (e.g. via AskUserQuestion) and post only what
they pick:

- **Which findings go inline.** Every finding tied to a `path:line` should normally be an inline
  comment; only cross-cutting commentary (summary, recommendation, SQA/docs overview) belongs in
  the review body. Confirm the set, and that small fixes (typos, grammar, formatting) will be
  posted as one-click **suggestion** blocks rather than prose.
- **Whether to post inline at all**, or body-only.
- **The review event** - default `COMMENT`; use `APPROVE`/`REQUEST_CHANGES` only if the user is a
  CCB member and explicitly asks.

Treat the body-plus-inline review as one deliverable: draft both, get approval, then post in one
action. Posting the body and deferring the inline comments ("want me to add those?") is not the
workflow.

**Preferred mechanism - one review carrying body + inline comments** (single API call, one
notification):

```bash
HEAD_SHA=$(gh api repos/{owner}/{repo}/pulls/<num> --jq .head.sha)
gh api repos/{owner}/{repo}/pulls/<num>/reviews --method POST --input review.json
```

`review.json`:
```json
{
  "commit_id": "<HEAD_SHA>",
  "event": "COMMENT",
  "body": "<report body, including the AI-disclosure footer>",
  "comments": [
    { "path": "framework/.../Foo.md", "line": 7, "side": "RIGHT",
      "body": "`!alert node` is not a valid alert type.\n\n```suggestion\n!alert note\n```\n\n_(generated by <model>)_" },
    { "path": "src/.../Foo.C", "start_line": 40, "line": 42, "side": "RIGHT",
      "body": "Mark this `const`. _(generated by <model>)_" }
  ]
}
```

Rules for `comments[]`:
- `line` is the line on the NEW side (`side: "RIGHT"`) and must fall inside a changed hunk - for
  entirely new files every added line qualifies. For **modified** files, confirm the target line
  was actually added or changed by the PR (a `+` line in the diff, or context inside a hunk); an
  unchanged line - even one right next to the change - is rejected. Get exact numbers from the PR
  head, **not** the diff's own line numbers (e.g. `git fetch origin pull/<num>/head` then
  `git show "FETCH_HEAD:<path>" | grep -n <pattern>`), and verify hunk membership before posting -
  the atomic POST fails entirely on one bad anchor.
- For a multi-line range set `start_line` + `line`.
- A **suggestion** block must reproduce the replacement for the commented line(s) verbatim -
  GitHub overwrites them with the block's contents, so match indentation exactly.
- The POST is atomic: one bad anchor rejects the whole review. When assembling many comments,
  build `review.json` from a small script so comment text never has to survive shell quoting.

**Fallbacks:**
- *Body only* (no line-anchored items): `gh pr review <num> --comment --body-file <draft.md>`.
- *Body already posted, adding inline afterward*: post a companion review the same way (its `body`
  can just say "line-level findings, companion to the summary above"), then **edit the original
  body** to point at it:
  `gh api repos/{owner}/{repo}/pulls/<num>/reviews/<review_id> --method PUT -f body="$(cat draft.md)"`.

Keep the tone respectful and helpful (MOOSE's conduct standard). The goal is to give the
author clear direction, not to implement the change for them.
