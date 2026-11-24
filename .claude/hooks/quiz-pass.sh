#!/bin/bash
# Record that the user passed the commit quiz for the currently staged tree.
# The token records "<staged tree> <HEAD>", so it unlocks exactly this content
# on top of exactly this history. Any further `git add` invalidates it, and so
# does the commit it authorizes -- which is what stops a second unquizzed commit
# while still allowing that commit to be amended.

#
# A token records whether the quiz was actually graded. --graded means the user
# answered three questions correctly; without it the unlock counts as an
# override and the commit it authorizes is marked in its message. Override is
# the default on purpose: this script cannot tell who ran it, so forgetting the
# flag must fail toward disclosure rather than away from it.

set -euo pipefail

graded=0
args=()
for a in "$@"; do
  case "$a" in
    --graded) graded=1 ;;
    *) args+=("$a") ;;
  esac
done

if [[ ${#args[@]} -lt 3 ]]; then
  echo "usage: quiz-pass.sh [--graded] '<question>|<answer>' x3" >&2
  echo "Pass the three questions and the user's answers, for the log." >&2
  echo "--graded asserts the user answered all three correctly; without it" >&2
  echo "the next commit is marked as having skipped the quiz." >&2
  exit 1
fi
set -- "${args[@]}"

git_dir=$(git rev-parse --git-dir)

if git diff --cached --quiet; then
  echo "Nothing is staged. Stage the change first, then re-quiz." >&2
  exit 1
fi

staged_tree=$(git write-tree)
head_now=$(git rev-parse HEAD 2>/dev/null) || head_now="unborn"
kind=$([[ "$graded" == 1 ]] && echo graded || echo override)
printf '%s %s %s\n' "$staged_tree" "$head_now" "$kind" \
  > "$git_dir/claude-commit-quiz-pass"

log="$git_dir/claude-commit-quiz.log"
{
  # Spelled out rather than using ${kind^^}, which needs bash 4; macOS has 3.2.
  label=$([[ "$graded" == 1 ]] && echo GRADED || echo OVERRIDE)
  echo "=== $label $(date -u +%Y-%m-%dT%H:%M:%SZ) tree=$staged_tree head=$head_now"
  for qa in "$@"; do
    echo "  $qa"
  done
} >> "$log"

echo "Quiz $kind recorded for staged tree $staged_tree on HEAD $head_now."
if [[ "$graded" == 0 ]]; then
  echo "This is an OVERRIDE: the next commit message will be marked"
  echo "\"Knowledge verification quiz was skipped\"."
fi
echo "This authorizes one commit, which may then be amended."
echo "The commit itself must still be run separately. Log: $log"
