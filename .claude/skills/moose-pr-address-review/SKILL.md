---
name: moose-pr-address-review
description: >-
  Address feedback on an existing MOOSE pull request by retrieving review
  threads and summary comments, classifying each item, applying accepted
  changes under MOOSE and AGENTS.md guidance, running verification, and
  drafting evidence-based replies. Use when someone asks to address PR
  comments, respond to a review, implement review feedback, or prepare a PR
  revision. Never resolve a non-trivial conversation; leave it to the reviewer.
---

# Addressing a MOOSE Pull Request Review

Address review feedback as a traceable loop from comment to code to
verification to reply. Local edits are part of this workflow; commits, pushes,
GitHub replies, and thread resolution require explicit approval after the user
has seen their exact contents.

Use `moose-codegraph` before changing MOOSE C++ or Python, and use the
read-only Steps 0-4 of `moose-pr-review` to re-audit the revised PR. Use
`moose-verify-changes` for build and test execution.

## Step 1 - Identify the PR and preserve local state

Resolve a number, URL, or current-branch PR to an exact repository, PR number,
base branch, head repository/branch, and head SHA. Confirm that the user can
modify the head branch. Record:

- The initial head SHA.
- `git status` and any preexisting working-tree changes.
- The PR's title, body, commits, changed files, checks, review decision, and
  requested reviewers.

Preserve unrelated changes. If they overlap review work or make attribution
ambiguous, stop and ask the user how to separate them. Do not stash, discard,
or overwrite them.

Check out or update the PR branch only when it is safe to do so. Do not use a
forceful checkout or reset.

## Step 2 - Retrieve every review conversation

Fetch all of these, following pagination:

- Review summaries and states.
- General PR issue comments.
- Review threads, including resolved and outdated threads.
- Every reply in each thread.

`gh pr view --json reviews,comments,...` does not provide the complete thread
model. Query the pull request's GraphQL `reviewThreads` connection and retain
each thread's ID, `isResolved`, `isOutdated`, path, line, URL, authors, bodies,
and replies. Resolved threads provide context but are never reopened or
modified.

## Step 3 - Classify before editing

Classify every outstanding item by disposition:

- **Accepted:** make the requested change.
- **Clarification needed:** draft a focused question and do not guess.
- **Disagreement:** preserve the code and draft a respectful, evidence-based
  rationale.
- **Already addressed:** collect the path, test, or commit evidence.
- **Obsolete:** explain why the current diff no longer contains the issue.

Also classify its resolution authority:

- **Trivial:** a deterministic spelling, grammar, whitespace, or formatting
  correction with no change to behavior, API, data, algorithm, build, test
  expectation, requirement, or design meaning.
- **Non-trivial:** everything else, including `const` or include changes,
  renames, test expectation changes, semantic comment edits, and any item that
  requires judgment. Treat uncertainty as non-trivial.

The resolution classification does not decide whether to implement a clear
request. It decides only who may mark the conversation resolved.

For an ambiguous non-trivial request or a meaningful design tradeoff, stop and
ask the user before editing. For a clear accepted request, continue.

## Step 4 - Apply accepted changes surgically

For each file to edit:

1. Read only the `AGENTS.md` files at the repository root and in directories
   between the repository root and the changed file. Never search above the
   repository root. Apply all non-conflicting instructions; the closest file
   takes precedence on a conflict.
2. Use `moose-codegraph` before changing MOOSE C++ or Python to find existing
   functionality, sibling patterns, callers, and blast radius.
3. Make only changes traceable to accepted review feedback. Do not clean up
   adjacent code.
4. Preserve correct preexisting comments. Remove a comment only when its code
   is removed, update it when the change makes it incorrect, or relocate it
   without losing its meaning.
5. Keep a mapping from each local change to its review thread or summary item.

Do not implement suggestions blindly. If evidence shows that a suggestion is
incorrect or already addressed, prepare a reply instead.

## Step 5 - Verify the revision

Use `moose-verify-changes` to verify the accepted changes, then use Steps 0-4
of `moose-pr-review` to re-audit the full PR. Check the delta from the recorded
starting SHA separately so unrelated or accidental changes are visible.

If a relevant check fails or is not run, keep the thread open and report the
limitation. Do not claim the feedback is addressed.

## Step 6 - Prepare the response package

Before any commit, push, reply, or resolution, show:

| Conversation | Classification | Action/evidence | Proposed reply | Resolution |
| --- | --- | --- | --- | --- |
| `<URL or ID>` | `<disposition>; <trivial/non-trivial>` | `<path/test/reason>` | `<exact text>` | `<leave open or eligible trivial resolution>` |

Append a concise disclosure to every proposed GitHub reply using the actual
model in use. Do not hardcode a model or client name. A summary review comment
that is not attached to a thread may receive one consolidated PR comment;
still show its exact text first.

Never stage, commit, amend, rebase, push, or force-push without explicit user
approval. Default to proposing one new commit containing the accepted review
changes. Ensure that at least one commit in the development branch references
an issue; do not add a redundant reference to the new commit when the branch
already satisfies that requirement. Show its exact files and message. Require
separate approval for amend, rebase, or force-push, and prefer `--force-with-lease`.

Local edits made while addressing the requested review are authorized; their
publication is not. Do not treat the initial request to "address the review"
as permission to publish replies or change remote state.

## Step 7 - Publish only what was approved

After approval:

1. Perform only the approved commit and push operations.
2. Recheck the remote head SHA before posting so replies describe the code
   that is actually visible on the PR.
3. Reply to a review thread with the GraphQL
   `addPullRequestReviewThreadReply` mutation, using its thread ID.
4. Post approved summary responses as PR comments.
5. Verify each posted reply and report its URL.

Replying and resolving are separate operations. Follow these hard rules:

- **Never resolve a non-trivial conversation.** Leave it open for the reviewer
  even when the requested change is implemented and verified. If asked to
  resolve it, explain that this workflow reserves that action for the reviewer.
- Resolve a trivial thread only after its fix passes verification and the user
  explicitly approves that exact thread in the response package. Use
  `resolveReviewThread` only for those enumerated thread IDs.
- Never infer resolution approval from approval to commit, push, or reply.
- Never reopen, reply to, or otherwise modify a thread the reviewer already
  resolved unless the user explicitly requests a new reply; do not unresolve
  it.

For an accepted change, reply only `Done.` unless a short qualifier is needed
to identify what changed. Do not ask the reviewer to confirm it or mention
that the conversation was left open. For disagreement or clarification, post
only the approved rationale or question and leave the thread open.

## Finish concisely

After publishing, return only a brief success note and the PR URL. Do not
repeat the response package or summarize commits, replies, and resolutions the
user can inspect on GitHub. Expand the response only for an action that failed,
was skipped, remains blocked, or still requires user input.
