#!/bin/bash
# PreToolUse(Bash) gate: refuse `git commit` until the agent has shown the user
# the commit message and quizzed them on the change's content.
#
# A pass token is written by quiz-pass.sh and records "<staged tree> <HEAD>", so
# it cannot be reused for different content than it was granted for.
#
# Amending the commit a quiz authorized is allowed: a reworded message or a
# fixup to the same content was already reviewed. A second plain commit is not,
# because that is new history the user never saw. The two cases are
# indistinguishable from repository state alone -- after a commit both have the
# same staged tree and the same HEAD -- so the gate reads the --amend flag and
# compares against the parent of HEAD.

set -uo pipefail

payload=$(cat)

# Decide whether any segment of the command line is a `git commit`. Checking for
# the bare word "commit" is not enough: it also appears as an option value, as
# in `git log --grep=commit`. So tokenize each shell segment and require that
# `commit` is the git SUBCOMMAND, after any global options.
printf '%s' "$payload" | python3 -I -c '
import json, re, shlex, sys

try:
    cmd = json.load(sys.stdin).get("tool_input", {}).get("command", "")
except Exception:
    sys.exit(0)

# Global git options that consume a following separate argument.
TAKES_ARG = {"-C", "-c", "--git-dir", "--work-tree", "--namespace", "--exec-path"}

def is_git_commit(tokens):
    i = 0
    # Step over leading env assignments and privilege wrappers.
    while i < len(tokens) and (
        re.fullmatch(r"[A-Za-z_][A-Za-z0-9_]*=.*", tokens[i])
        or tokens[i] in ("sudo", "env", "command", "nohup", "time")
    ):
        i += 1
    if i >= len(tokens):
        return False
    exe = tokens[i].rsplit("/", 1)[-1]
    if exe not in ("git", "git.exe"):
        return False
    i += 1
    # Step over git global options to reach the subcommand.
    while i < len(tokens):
        t = tokens[i]
        if not t.startswith("-"):
            break
        if t in TAKES_ARG:
            i += 2
        else:
            i += 1
    if i >= len(tokens) or tokens[i] != "commit":
        return False
    # Report whether this commit is an amend, so the gate can allow reworking
    # the commit a quiz already authorized.
    return "amend" if any(t == "--amend" for t in tokens[i + 1:]) else "plain"

# Split on shell separators, then test each segment independently.
verdict = None
for segment in re.split(r"(?:\|\||&&|[;|&\n])", cmd):
    segment = segment.strip()
    if not segment:
        continue
    try:
        tokens = shlex.split(segment)
    except ValueError:
        tokens = segment.split()
    kind = is_git_commit(tokens)
    if kind == "plain":
        verdict = "plain"
        break
    if kind == "amend":
        verdict = "amend"

if verdict == "plain":
    sys.exit(1)
if verdict == "amend":
    sys.exit(2)
sys.exit(0)
'
case $? in
  0) exit 0 ;;   # not a commit at all
  1) is_amend=0 ;;
  2) is_amend=1 ;;
esac

git_dir=$(git rev-parse --git-dir 2>/dev/null) || exit 0
token="$git_dir/claude-commit-quiz-pass"

staged_tree=$(git write-tree 2>/dev/null) || staged_tree="unresolvable"
head_now=$(git rev-parse HEAD 2>/dev/null) || head_now="unborn"

if [[ -f "$token" ]]; then
  read -r tok_tree tok_head _ < "$token"

  if [[ "$is_amend" == 0 ]]; then
    # A plain commit needs the token to match the content being committed, and
    # to predate it: once the quizzed commit exists, HEAD has moved past the
    # recorded one and the token no longer authorizes new history.
    if [[ "$tok_tree" == "$staged_tree" ]] && [[ "$tok_head" == "$head_now" ]]; then
      exit 0
    fi
  else
    # An amend replaces HEAD, so the token must have authorized HEAD itself --
    # i.e. been issued against HEAD's parent. The staged tree must still be the
    # reviewed one: amending is for rewording or re-running the same content,
    # not for folding in code the quiz never covered.
    head_parent=$(git rev-parse HEAD^ 2>/dev/null) || head_parent="root"
    if [[ "$tok_head" == "$head_parent" ]] && [[ "$tok_tree" == "$staged_tree" ]]; then
      exit 0
    fi
  fi
fi

cat >&2 <<'EOF'
BLOCKED: commit quiz gate. Do not retry this command, and do not work around it
with `git commit` variants, plumbing (`git commit-tree`, `git update-ref`), or a
background shell. Instead, do the following in order:

1. Show the user the full, final commit message you intend to use, verbatim, in
   your reply text. Not a summary of it.

2. Ask the user three multiple-choice questions in ONE AskUserQuestion call.

   Ask about CONSEQUENCES, not about code trivia. Every question must describe a
   situation the user could actually hit and ask what this change does about it.
   The reader should be able to answer from understanding the change, without
   re-reading the diff line by line. Good shapes:
     - a concrete scenario: "You do X, then Y happens -- what does the code do?"
     - blast radius: "If this is wrong at run time, what breaks, and for whom?"
     - recovery: "After this fails, what is lost and what do you still have?"
     - honest limits: "What does this NOT protect against, as built?"

   Do NOT ask the user to simulate an edit to the source. No "if line N said
   `||` instead of `&&`", no "if this set were deleted", no predicting the
   behavior of code that is not in the diff. That tests whether someone can
   hand-execute a mutation, not whether they understood the change, and it is
   unfair even when the mutated line is visible in the diff.

   Prefer the questions whose wrong answer would cost the user something real.
   If a question's answer would not change how the user reviews, uses, or
   trusts this change, cut it and ask a better one.

   Each question gets one correct option and plausible wrong ones describing
   outcomes that genuinely could have been the design. Do NOT hint at the answer
   in the phrasing, do NOT mark the correct option, and do NOT order the options
   so the correct one is always first. Never put "(Recommended)" on a quiz
   option. Keep every option about the same length -- a conspicuously detailed
   option gives the answer away.

3. Grade the answers.

   ALL THREE CORRECT -> run:
       .claude/hooks/quiz-pass.sh --graded "<q1>|<their answer>" "<q2>|<their answer>" "<q3>|<their answer>"
   then re-run your original git commit command.

   Pass --graded ONLY when the user answered all three correctly on their own.
   Not when they asked you to skip the quiz, not when you told them an answer,
   and not when you decided a miss was close enough. Without --graded the
   commit message gets the line "Knowledge verification quiz was skipped",
   which is the honest record of what happened -- do not strip it, and do not
   reach for --graded to avoid it.

   ANY WRONG -> tell the user plainly which answers were wrong and what the
   correct answers were, then run:
       .claude/hooks/quiz-fail.sh
   This parks the work on a quiz-failed/<timestamp> branch and resets tracked
   files. Report the recovery command the script prints. Do not commit, and do
   not re-quiz on the same work unless the user explicitly asks you to.

If the user's answer reveals that YOUR commit message or your own understanding
was the thing that was wrong, say so, fix the change or the message, and start
over at step 1. Do not grade them wrong for catching your error.
EOF

exit 2
