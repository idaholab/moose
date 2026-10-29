---
name: newsletter-write
description: >-
  Fill in a MOOSE monthly newsletter file under modules/doc/content/newsletter from the merge
  history of a given month, or grammar-edit an existing one. Use when someone asks to write,
  complete, update, or proofread a MOOSE newsletter, or to add entries for changes that merged
  during a particular month or date range.
---

# Writing a MOOSE monthly newsletter

Newsletter files live at `modules/doc/content/newsletter/<YYYY>/<YYYY>_<MM>.md`. Each one
summarizes what merged into MOOSE during that month, for readers who were not in any of the
pull request discussions. The audience is peer computational scientists and engineers.

`AGENTS.md` section 9 ("Writing MOOSE Contribution Materials") governs the prose: lead with the
result, keep MOOSE terminology and exact object/parameter names, explain unfamiliar terms, and
leave out investigation dead ends, "future work" catalogs, and stock machine-drafted phrasing.

## Step 1 - Establish the time frame

The month is the one piece of information this task cannot infer. Resolve it before anything else:

- If the invocation names a month, date range, or file (`/newsletter-write October 2026`,
  `2026_10.md`, `since 2026-10-01`), use that.
- Otherwise ask which month or date range to cover, and wait for the answer.

Bound the range on the **first day of the month through the first day of the next month**. Do not
widen it to catch merges from the preceding days; a change that merged in September belongs to
September's newsletter even if it was reviewed in October.

```bash
git log --merges --first-parent --since=<YYYY>-<MM>-01 --until=<YYYY>-<MM+1>-01 \
  --date=short --pretty='%h %ad %s'
```

`--first-parent` keeps this to the merges onto the release branch, one per pull request. Expect
on the order of 100-150 for a full month.

## Step 2 - Read the style authority

Read the **two most recent completed newsletters** before writing a word. They outrank anything
in this skill where they disagree, because conventions drift. Note in particular:

- Which heading level each section uses; `###` and `####` nesting has drifted over time.
- How bullets cite pull requests, and whether a section is prose or bullets.
- An unfilled month file, if one already exists, is the clean template: take the current
  top-level headings and their order from it, or from the most recent completed month. Leave any
  in-progress alert block at the top of the file alone.

If the target file already has entries, **those are not yours to rewrite.** Skip any change that
is already covered and move to the next one. Ask before modifying or regrouping existing text,
including when a new entry would read better merged into an existing one.

## Step 3 - Gather context per pull request

Commit subjects alone are not enough; they describe steps, not outcomes. For each merge, pull the
pull request title and body, then the referenced issue where the body is just `Closes #NNNNN`.

```bash
git log --no-merges --pretty='  - %s' <merge>^1..<merge>^2   # commits in one PR
gh pr view <N> --repo idaholab/moose --json number,title,body
gh issue view <N> --repo idaholab/moose --json title,body
```

`gh` may not be on `PATH`; it is often inside a conda environment
(`<conda-prefix>/envs/<env>/bin/gh`). Ask the user for its location rather than assuming it is
absent. Batch these calls in loops over the PR numbers; they are the bulk of the work.

Read each body whole rather than truncating it to a preview length. Where a body follows the
repository's `Reason`/`Design`/`Impact` template, the user-visible consequence - a carve-out, a new
requirement, a count, a renamed object - tends to sit in the later sections; many bodies do not
follow it, and then there is no predicting which part carries that detail. Either way, a
preview-length cut drops what the entry needs often enough that it is not worth doing.

For a new object or capability, also confirm what was actually added:

```bash
git diff --name-status <merge>^1 <merge> | grep -vE '/gold/'
```

New `doc/content/source/**.md` files name the objects worth linking, and new `include/`/`src/`
files confirm class names. The new documentation page is usually the clearest statement of what the
object does.

Read the diff of any change a user could hit, not only when the body is thin. A body written for a
reviewer assumes context a newsletter reader does not have, and it routinely omits the part worth
reporting. Look specifically for:

- an exclusion or carve-out the body does not mention, and the reason for it;
- a new requirement or error an existing input can now trip;
- the specific wrong value, vector, or branch a fix replaced, rather than that one was wrong.

```bash
git diff <merge>^1 <merge> -- <paths from --name-status>   # the behavior change itself
```

When the body is a bare `closes #NNNNN` the diff is the only source; when the body is detailed, the
diff is what confirms it and supplies the mechanism.

A body is written before and during review, so an object name in it may not be the name that
merged: names shift as a design is revised, and a body is rarely updated to match. Confirm every
name against its `registerMooseObject` call or its documentation page, and confirm a count or
measurement against the diff.

## Step 4 - Decide what earns an entry

A reader wants changes that affect their inputs, results, or workflow. Include new objects and
parameters, changed defaults and results, fixed incorrect behavior, performance work, and
developer-facing tooling or API changes.

Leave out:

- **Test-only changes**: added job slots, loosened or tightened tolerances, regolds, mesh-mode
  pins, making a test heavy, skipping a test under valgrind.
- **A change reverted within the same month.** Report the net effect, which is often nothing - but
  check the diff of both PRs before concluding that, because a partial revert leaves a residue
  worth one line.
- **Pure SQA metadata backfills**, unless a module's coverage is the point of the entry.
- **Investigation dead ends**, per `AGENTS.md` section 9.

Each distinct change earns one entry. Most pull requests contain exactly one, but before setting a
PR aside as covered, check whether it also did something unrelated that a reader would want: the
monthly package update that also adds a developer skill is one PR with two separate subjects, and
placing the version bumps does not account for the skill. Each such subject goes in the section
where it belongs.

Never cite one PR twice for the same change, or split one change's detail across sections. A single
change spanning two domains gets one entry that treats it in full, placed where most of its
substance lives, or in the month's catch-all section when it sits in neither.

## Step 5 - Group entries under headings

Two kinds of heading coexist, and the distinction matters:

**Recurring navigation headings** keep their exact names month to month so readers can follow a
subsystem across releases. The test is empirical, not a fixed list: if a heading also appears in
the months read in Step 2, it is navigation rather than description, so reuse its exact wording and
do not rewrite it into a summary. Subsystem sections, tooling and documentation sections, catch-all
sections, and the module names under `MOOSE Modules Changes` are typically of this kind. Which
subsystems appear varies by month, so check the files rather than assuming.

**Explanatory headings** are for a section covering one change, or several changes serving one
goal. Write these as a single-line summary in sentence case rather than a bare noun phrase naming
the topic. A heading of this kind has a subject and a verb, and states what is now true:

- `<thing> now <does what it did not before>` for a changed behavior or default.
- `<thing> can now <be used how>` for a new capability or option.
- `New <thing> for <purpose>` for a new object, when the purpose is what a reader is scanning for.

Say which object, parameter, or subsystem changed, using the name a user would type. A heading that
could sit unchanged over a different month's section is too vague to be worth the words.

Keep them under roughly 75 characters. If the summary runs longer, move the qualifying detail into
the body's first sentence, where it is needed anyway.

A heading over a group of *unrelated* fixes cannot honestly summarize them. Name the area the
fixes share and mark it as a fix group, by appending a word such as "fixes" or "robustness" to the
area name, which is how recent months title these sections.

## Step 6 - Write the entries

Lead with what changed and why it matters, then the mechanism, then the consequence for a user.
A good bug fix entry states the wrong behavior, the cause, and what results change.

Cite every change, with the citation starting its own line rather than trailing the last sentence.
This holds even when it would fit on the preceding line; inside a bullet, indent it to the bullet's
continuation. The form is:

```markdown
([idaholab/moose#NNNNN](https://github.com/idaholab/moose/pull/NNNNN))
```

Use `/pull/<N>` for a pull request and `/issues/<N>` for an issue, and match the URL to the number.
Several PRs for one entry go in one parenthetical, comma-separated across lines.

Conventions this file's prose follows:

- Link objects as `[ClassName.md]`, syntax blocks as `[Name](syntax/Name/index.md)`, and
  parameters as `[!param](/Block/Type/param_name)`.
- Serial (Oxford) comma; it dominates these files.
- ASCII only, per `AGENTS.md` section 4. Write "-" rather than an en dash.
- State that results change where they do, in those words; avoid "bit-for-bit".
- Say what a flag or parameter is called exactly as a user would type it.

## Step 7 - Verify before reporting done

Every `[ClassName.md]` link must resolve, or the documentation build breaks:

```bash
grep -oE "\[[A-Za-z0-9_]+\.md\]" <newsletter> | tr -d '[]' | sort -u | while read m; do
  find framework/doc modules/*/doc -name "$m" -path '*content*' | head -1 | grep -q . \
    && echo "OK  $m" || echo "MISSING  $m"
done
```

Base classes, interfaces, and many AD template variants have no page; write those as inline code
instead of a link. Also confirm that each `syntax/.../index.md` path exists under
`framework/doc/content/`, and that each `[!param]` path names the block and type that really
declares the parameter (`grep` the `addParam` call, or copy the path from the module's own
documentation).

Then re-check the heading structure, that no pre-existing entry changed, and that the file is
pure ASCII (`LC_ALL=C grep -n '[^ -~]' <newsletter>`).

## Grammar-edit mode

When asked to proofread rather than write, the same style authority applies: match the sibling
files in the same directory. Fix agreement, articles, parallel list structure, serial commas,
hyphenation, and non-ASCII characters; expand `A+B+C` shorthand into prose; move a trailing
citation to match the file's convention.

Do not rewrite technical content, reorganize sections, or delete entries under the guise of a
grammar pass. `AGENTS.md` section 3 applies: every changed line traces to the request. When a
sentence seems wrong on the facts rather than the grammar, raise it instead of silently fixing it.
