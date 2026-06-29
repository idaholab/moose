---
name: moose-pr-review
description: >-
  Review a MOOSE pull request or local branch against MOOSE's contribution standards:
  the MOOSE Code Standard (SCS), the SQA testing rules (every test is a requirement, with
  requirement/design/issues), required documentation stub pages for new objects, and PR
  governance (issue references, scope, required-vs-suggested phrasing). Use this whenever
  someone wants to review MOOSE changes or asks "review this PR", "review my branch",
  "is this ready to merge", "check my changes before I submit", or wants pre-submission
  feedback on code in the MOOSE framework or its modules - even if they never say the word
  "review". This is the MOOSE-aware review layer; it complements the generic /code-review
  and /security-review skills and defers to them for deep correctness and security analysis
  rather than duplicating that work.
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

## What CI already enforces (do not nitpick by hand)

Formatting is checked automatically on every PR, so manual review time should not be spent on
it. If you see violations, note them once and point to the fix command rather than listing
each instance:

- C++ formatting: `git clang-format <base>` (uses `.clang-format`)
- Python formatting: `black .` (uses `pyproject.toml`)
- Trailing whitespace / tabs, and the "every commit references an issue" check

Focus human/AI review on the semantic standards CI can't see: naming, const-correctness,
access control, API design, C-vs-C++ construct choices, tests, and documentation.

## Step 0 - Preflight: CodeGraph is required (do not skip)

This review depends on CodeGraph to detect code that reinvents functionality MOOSE already has.
That is the single most valuable thing this review catches against MOOSE's "use existing
functionality, don't reimplement it" rule (AGENTS.md, Simplicity First), and it is something
grep cannot do reliably across a 100k-symbol codebase. CodeGraph is therefore a hard
prerequisite, not an optimization.

Before anything else, confirm the index exists: check for a `.codegraph/` directory at the
repository root (or run `codegraph status`).

If it is missing, **stop and surface it to the user - do not fall back to grep and proceed.**
Tell them CodeGraph is required for a MOOSE review and how to set it up:

```bash
codegraph install   # one-time: register the CodeGraph MCP server with this agent
codegraph init      # build the index for this repo (run from the repo root; takes a few minutes)
```

Only continue once the index is present. Throughout the review, prefer the CodeGraph MCP tools
(`codegraph_explore`, `codegraph_search`, `codegraph_node`, `codegraph_callers`) over grep/Read
whenever the question is "does this already exist?", "what calls this?", or "what does changing
this affect?" See `references/codegraph.md` for the exact query patterns.

## Step 1 - Identify what you are reviewing

Determine the target and compute the diff. Two cases:

**A GitHub PR** (user gives a number or URL, or asks to review a specific PR):

```bash
gh pr view <num> --json title,body,baseRefName,headRefName,author,files,commits,state,statusCheckRollup
gh pr diff <num>
```

`baseRefName` is the merge target (MOOSE PRs target `next`). Use the PR body to read the
author's stated Reason / Design / Impact (the PR template fields).

**A local branch** (the default when no PR is named):

```bash
# Review the branch against the point it was cut from. <base> is the integration branch the
# work branched from (its upstream tracking branch, e.g. origin/devel). Use three dots so the
# diff shows only what this branch added, regardless of how far the integration branch moved.
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

- **Linked issue(s).** Read the issue(s) the commits reference - they are your spec for judging
  whether the change is complete and correctly scoped. (CI already enforces that a reference
  exists, so don't spend review time policing its presence.)
- **Newly registered objects.** Find new user-facing objects in the diff:
  `git diff <base>...HEAD | grep -E '^\+.*register(MooseObject|.*Action)\('`.
  Each new object drives both a documentation check (Step 3E) and a testing check (Step 3D).
- **Scope.** Is the change surgical and focused, or does it bundle unrelated edits ("rider"
  commits)? MOOSE reviewers may legitimately ask to split a sprawling PR. Note mixed concerns.

## Step 3 - Review across the MOOSE dimensions

Work top-down: assess the high-level design first (a sound design with rough edges is fixable;
a wrong design is not), then the details. For each dimension, the highest-value checks are
summarized here; the reference files hold the full checklists - read the relevant one when the
diff touches that area.

**A. Design, scope, and user interface** (`reviewing.md`)
- Is the high-level design sound, and proportionate scrutiny applied (changes affecting many
  users or that others will build on deserve more)?
- Scrutinize the **user interface**: new input parameters, their names, defaults, and whether
  `validParams` documentation strings make them self-explanatory. Interface mistakes are
  expensive because users depend on them.
- Will the code be understandable outside the PR context, from comments and docs alone?

**B. Reuse and duplication (CodeGraph-driven - vital)** -> read `references/codegraph.md`
- For every new function, class, or method the PR introduces, ask CodeGraph whether MOOSE
  already provides it. Reinventing an existing utility, algorithm, or object is one of the most
  common and most important things to catch - it bloats the codebase and diverges behavior.
- Workflow: extract the new symbols from the diff, then `codegraph_search` them by name and
  `codegraph_explore` the concept to surface existing equivalents. If a near-duplicate exists,
  flag it as required and point the author to reuse or extend it (a short extension/bugfix of
  existing code is preferred over a long reimplementation).
- Also use `codegraph_callers` / `impact` on changed existing symbols to judge blast radius
  (this feeds the scope/scrutiny assessment in A) and to find call sites the PR should update.

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
  `framework/doc/content/source/kernels/Foo.md`). A missing stub page fails the docs build.
- Significant changes warrant a newsletter entry (current month, e.g.
  `modules/doc/content/newsletter/2026/2026_06.md`).

**F. Correctness and security** (defer, don't duplicate)
- For logic bugs, edge cases, and efficiency, run the built-in `/code-review` (or recommend it).
- For anything security-sensitive (parsing untrusted input, file/system access, memory safety),
  run or recommend `/security-review`. Memory-management-heavy code may warrant a valgrind
  testing recipe; loose tolerances often break parallel testing - mention these when relevant.

## Step 4 - Write the report

The central organizing principle of a MOOSE review is separating what the author **must**
change from what you **suggest**. Be explicit about which is which (per `reviewing.md`):
required items use imperative phrasing ("Mark this parameter `const`"); suggestions use "I
suggest" / "consider". Treat minor items (typos, grammar, docstrings) as required. Every
finding should be actionable and anchored to a `path:line` so the author can jump to it.

Use this structure:

```
# MOOSE PR Review: <title or branch name>

## Summary
<2-4 sentences: what the change does, overall design assessment, and a recommendation
(approve / approve-with-nits / changes-required).>

## Required changes
1. `path/to/file.C:42` - <what is wrong and why it must change> _(standard | tests | docs | design)_
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
  entirely new files every added line qualifies. Get exact numbers from the PR head, **not** the
  diff's own line numbers (e.g. `git fetch origin pull/<num>/head` then
  `git show "FETCH_HEAD:<path>" | grep -n <pattern>`).
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
