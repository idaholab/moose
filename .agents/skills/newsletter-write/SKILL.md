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
widen it to catch merges from the preceding days. The merge date decides the month, not the review:
a pull request reviewed through late September but merged on October 2 belongs to October.

```bash
git log --merges --first-parent --since=<YYYY>-<MM>-01 --until=<YYYY>-<MM+1>-01 \
  --date=short --pretty='%h %ad %s'
```

`--first-parent` keeps this to the merges onto the release branch, one per pull request. Expect
on the order of 100-150 for a full month.

Fetch the base branch and work from it, rather than from a local copy that may be behind. A month's
newsletter is written days after the month closes, and in that window other authors add entries to
the target file and reviewers merge changes that affect what belongs in it. A stale base silently
costs both: entries written by other people go missing when the branch is rebased, and the month's
merge list is incomplete.

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

A PR's comments can supersede its body - a reviewer or the author may have narrowed a claim after
the description was written, for instance by establishing that a stated benefit only partially
applies. Skim the thread (`gh pr view <N> --json comments`) for this kind of correction before
citing the body's framing, especially for a performance or capability claim.

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
- **Documentation typo, link, and wording corrections**, and renames of internal classes or
  methods that no input or application code names.
- **Cosmetic web and tooling changes** a reader cannot act on, such as scrollbar behavior or a URL
  change in file headers.

An entry belongs in `MOOSE Bug Fixes` only when a user's existing, documented-as-working input or
workflow produced a wrong result. A new capability that happens to close an issue, such as adding
support for a case that was never claimed to work, or a change to test or CI infrastructure rather
than framework behavior, belongs in `MOOSE Improvements` even if its issue or PR title says "fix".

Before writing an entry that frames something as completed or backfilled, check for an open PR or
issue referencing the same tracking issue as the merged work. If the completion claim depends on
unmerged work, such as a follow-up enforcing a convention the merged PRs only backfilled, wait for
it to merge or drop the entry rather than overstating what is actually done.

Most of what merges in a month does not earn an entry. A full month's 100-150 merges yield roughly
60-90 entries, so expect to set aside close to two in five PRs without writing anything for it.

Each distinct change earns one entry, and most pull requests contain exactly one. A PR does not
earn more than one by touching several files; it earns a second entry only when it did a second
thing a reader would look for separately, as the monthly package update that also adds a developer
skill did. Before setting a PR aside as covered, check for such a subject; each one goes in the
section where it belongs.

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
could sit unchanged over a different month's section is too vague to be worth the words. The point
of the forms above is the specific name, not the sentence shape: a heading built from the shape
alone, with the name left out, is worse than the bare noun phrase it replaced.

Keep them under roughly 75 characters. If the summary runs longer, move the qualifying detail into
the body's first sentence, where it is needed anyway.

Check that the heading's claim is no broader than its constituent PRs. Rolling two narrow fixes
under one heading can drift the claim from "what these PRs did" to a general property of the
system they did not actually establish; name the specific systems or objects they touched instead
of a system-wide claim neither PR earns.

A heading over a group of *unrelated* fixes cannot honestly summarize them. Name the area the
fixes share and mark it as a fix group, by appending a word such as "fixes" or "robustness" to the
area name, which is how recent months title these sections.

**Features come before fixes.** The file's two top-level sections, `MOOSE Improvements` and
`MOOSE Bug Fixes`, already carry this split, so put a fix under the latter unless it belongs with a
feature a reader would read alongside it. A subsystem section that must hold both, such as a
backend whose month included new capability and corrections, orders its entries features first and
fixes last rather than interleaving them. Do not add "Features" and "Fixes" subheadings to do this;
ordering carries it, and the newsletters use no such headings.

Judge a section by how a reader skimming it would rank what they find: when more than about a third
of a feature section's entries describe something that used to be wrong, the fixes are in the wrong
place.

## Step 6 - Write the entries

Lead with what changed and why it matters, then the mechanism, then the consequence for a user.
A good bug fix entry states the wrong behavior, the cause, and what results change.

### Length

**Budget about 40 words per entry across the framework and module sections**, and measure it rather
than trusting the impression that each entry reads tightly; they do individually, and the total is
still too long. Take the figure from the two months read in Step 2 instead of this number if they
disagree:

```bash
python3 -I -c "
import re,sys
t=open(sys.argv[1]).read()
i=t.find('## libMesh-level Changes')          # absent in some months
b=t[:i] if i > 0 else t
w=len(b.split()); n=len(set(re.findall(r'moose/(?:pull|issues)/([0-9]+)',b)))
print(f'{w} words, {n} entries, {w/n:.0f} words/entry')
" <newsletter>
```

The package and libMesh sections sit outside this measure; they are supplied, not written here.

Most entries are one or two sentences. Spend four or five only on a change that reshapes how an
input is written or what a result means, and expect two or three such items in a month, not twenty.
When the budget is exceeded, cut in this order:

1. **Drop entries that do not clear Step 4.** A shorter list of things that matter beats a complete
   list of everything that merged.
2. **Cut the mechanism from entries whose outcome stands alone.** A reader needs the internal call
   path only when it tells them which of their inputs is affected.
3. **Compress the remaining prose.** Do this last, and only on entries whose content has already
   been judged worth keeping; tightening sentences cannot recover a 40% overrun on its own.

### Register

Write for a reader deciding whether a change affects their work, not for the reviewer of the pull
request it came from:

- **Name what a user can name.** Objects, parameters, input blocks, and command-line flags belong
  in an entry because a reader can search for them. An internal class, a method, a member variable,
  or a source file name usually does not, since it tells them nothing they can act on. Reach for
  one only when the reader has no other handle on the change, as with an API an application
  subclasses.
- Vary the construction. A whole section of "X gained a parameter" reads as generated; prefer "X
  takes a new `param`", "`param` selects ...", or stating what the parameter does.
- Expand an abbreviation or internal term on first use if a reader outside the PR's discussion
  would not recognize it, such as "COOrdinate-format" rather than bare "COO". This is distinct
  from naming objects and parameters - it is about not assuming the reader was in the review.

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
- Serial (Oxford) comma, which these files use throughout.
- ASCII only, per `AGENTS.md` section 4. Write "-" rather than an en dash.
- State that results change where they do, in those words; avoid "bit-for-bit". A Jacobian-only fix
  changes convergence behavior and iteration count, not the converged answer - do not say "results
  change" unless the fix is in the residual or in a value a user reads out.
- Say what a flag or parameter is called exactly as a user would type it.

## Step 7 - Verify before reporting done

Every `[ClassName.md]` link must resolve, or the documentation build breaks:

```bash
grep -oE "\[[A-Za-z0-9_]+\.md\]" <newsletter> | tr -d '[]' | sort -u | while read m; do
  find framework/doc modules/*/doc -name "$m" -path '*content*' | head -1 | grep -q . \
    && echo "OK  $m" || echo "MISSING  $m"
done
```

Base classes and interfaces have no page; write those as inline code. An AD variant usually has no
page of its own either. Check for one anyway, because some do: where it exists, link it, and where
it does not, link the non-AD object it mirrors and leave the AD class as inline code beside it.

Also confirm that each `syntax/.../index.md` path exists under
`framework/doc/content/`, and that each `[!param]` path names the block and type that really
declares the parameter (`grep` the `addParam` call, or copy the path from the module's own
documentation).

Then re-check the heading structure and that the file is pure ASCII
(`LC_ALL=C grep -n '[^ -~]' <newsletter>`).

Measure the length against the budget in Step 6 and against the two months read in Step 2. Writing
tight sentences is not the same as writing a newsletter of the right length, and the difference
only shows up in the count.

Prefer claims the PR's own diff or body establishes. A true fact learned by exploring adjacent
code the PR did not change is usually not worth the length or the extra verification burden it
adds - drop it unless a reader cannot understand the entry's scope without it.

Finally, confirm that no entry anyone else wrote was changed or lost, by diffing against the base
branch rather than against the file as it looked when the work started:

```bash
git fetch <remote> <base> && git diff <remote>/<base>...HEAD -- <newsletter> | grep '^-'
```

Every removed line should be one this work deliberately removed. A pre-existing entry appearing
there means a rebase or a rewritten section dropped someone else's work.

## Grammar-edit mode

When asked to proofread rather than write, the same style authority applies: match the sibling
files in the same directory. Fix agreement, articles, parallel list structure, serial commas,
hyphenation, and non-ASCII characters; expand `A+B+C` shorthand into prose; move a trailing
citation to match the file's convention.

Do not rewrite technical content, reorganize sections, or delete entries under the guise of a
grammar pass. `AGENTS.md` section 3 applies: every changed line traces to the request. When a
sentence seems wrong on the facts rather than the grammar, raise it instead of silently fixing it.
