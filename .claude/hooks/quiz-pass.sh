#!/bin/bash
# Record that the user passed the commit quiz for the currently staged tree.
# The token is bound to the staged tree hash, so it unlocks exactly this content
# and nothing else. Any further `git add` invalidates it.

set -euo pipefail

if [[ $# -lt 3 ]]; then
  echo "usage: quiz-pass.sh '<question>|<answer>' x3" >&2
  echo "Pass the three graded questions and the user's answers, for the log." >&2
  exit 1
fi

git_dir=$(git rev-parse --git-dir)

if git diff --cached --quiet; then
  echo "Nothing is staged. Stage the change first, then re-quiz." >&2
  exit 1
fi

staged_tree=$(git write-tree)
printf '%s' "$staged_tree" > "$git_dir/claude-commit-quiz-pass"

log="$git_dir/claude-commit-quiz.log"
{
  echo "=== PASS $(date -u +%Y-%m-%dT%H:%M:%SZ) tree=$staged_tree"
  for qa in "$@"; do
    echo "  $qa"
  done
} >> "$log"

echo "Quiz pass recorded for staged tree $staged_tree."
echo "The commit will go through now. Log: $log"
