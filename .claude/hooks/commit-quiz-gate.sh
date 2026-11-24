#!/bin/bash
# PreToolUse(Bash) gate: refuse `git commit` until the agent has shown the user
# the commit message and quizzed them on the change's content.
#
# A pass token is written by quiz-pass.sh and bound to the hash of the staged
# tree, so it cannot be reused for different content than it was granted for.

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
    return i < len(tokens) and tokens[i] == "commit"

# Split on shell separators, then test each segment independently.
for segment in re.split(r"(?:\|\||&&|[;|&\n])", cmd):
    segment = segment.strip()
    if not segment:
        continue
    try:
        tokens = shlex.split(segment)
    except ValueError:
        tokens = segment.split()
    if is_git_commit(tokens):
        sys.exit(1)
sys.exit(0)
' && exit 0

git_dir=$(git rev-parse --git-dir 2>/dev/null) || exit 0
token="$git_dir/claude-commit-quiz-pass"

staged_tree=$(git write-tree 2>/dev/null) || staged_tree="unresolvable"

if [[ -f "$token" ]] && [[ "$(cat "$token")" == "$staged_tree" ]]; then
  exit 0
fi

cat >&2 <<'EOF'
BLOCKED: commit quiz gate. Do not retry this command, and do not work around it
with `git commit` variants, plumbing (`git commit-tree`, `git update-ref`), or a
background shell. Instead, do the following in order:

1. Show the user the full, final commit message you intend to use, verbatim, in
   your reply text. Not a summary of it.

2. Ask the user three multiple-choice questions in ONE AskUserQuestion call.
   The questions must be answerable only by someone who actually understands
   this change. Draw them from the real content of the diff, for example:
     - which function or behavior changes, and in which direction
     - what observably breaks, or what wrong result appears, without this change
     - why a specific choice in the diff was made over the obvious alternative
     - what the test added here would catch that the pre-existing tests did not
   Each question gets one correct option and plausible wrong ones taken from
   nearby real code, not filler. Do NOT hint at the answer in the phrasing, do
   NOT mark the correct option, and do NOT order the options so the correct one
   is always first. Never put "(Recommended)" on a quiz option.

3. Grade the answers.

   ALL THREE CORRECT -> run:
       .claude/hooks/quiz-pass.sh "<q1>|<their answer>" "<q2>|<their answer>" "<q3>|<their answer>"
   then re-run your original git commit command.

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
