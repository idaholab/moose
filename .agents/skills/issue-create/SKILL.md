---
name: issue-create
description: >-
  Draft and, only after explicit approval, open a GitHub issue on idaholab/moose that follows
  the repository's issue templates (feature request or bug report). Use when someone asks to
  create, open, or file a MOOSE issue, or when a commit or pull request needs an issue to
  reference and none exists yet.
---

# Creating a MOOSE issue

MOOSE issues follow the forms in `.github/ISSUE_TEMPLATE/`. `gh issue create` bypasses those
forms, so reproduce their section headings and labels in the issue body yourself. Read the
template files before drafting; they are the source for the headings and labels below, and they
win if they have changed.

## Step 1 - Check for an existing issue

Search open and closed issues for the same subject (`gh issue list -R idaholab/moose --search
'<terms>' --state all`). If one already covers it, return its number instead of filing a
duplicate.

## Step 2 - Pick the template

- `feature-request.yml` ("Feature or Enhancement Request", label `T: task`): new capability,
  enhancement, refactor, changed default, cleanup, or documentation work. Most issues filed
  alongside a pull request are this kind.
- `bug-report.yml` ("Bug Report", labels `T: defect` and `P: normal`): existing behavior is
  wrong and can be reproduced. A missing feature is not a bug.
- Questions go to GitHub Discussions, not issues.

## Step 3 - Draft the body

Use the template's field labels as `###` headings, in the template's order, and fill every
required field. The form's optional diagnostics field may be omitted.

Feature request:

```
### Motivation

### Design

### Impact
```

Bug report:

```
### Bug Description

### Steps to Reproduce

### Impact
```

Write the content under the contribution-material rules in AGENTS.md section 9: lead with the
problem and why it matters, keep it concise, preserve exact class, parameter, and command names,
and state numbers a reader can check. Under Impact, say whether existing APIs, defaults, or input
syntax change.

Add a component label (`C: <Area>`, e.g. `C: TestHarness`, `C: Modules/Heat Transfer`) when the
area is clear; list existing labels with `gh label list -R idaholab/moose --limit 300`.

## Step 4 - Approval, then publication

Show the title, labels, and body, and wait for explicit approval before creating the issue. Then
create it with every label from the template plus the component label:

```bash
gh issue create -R idaholab/moose --title '<title>' --label 'T: task' --label 'C: <Area>' \
  --body-file <file>
```

Return the issue URL. When the issue backs a commit or pull request, reference it as `Refs #<n>`
or `Closes #<n>` there.
