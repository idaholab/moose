#!/bin/bash
# Park the current work on a quiz-failed/<timestamp> branch and reset tracked
# files. Nothing is destroyed: the parked branch is a real ref, so the work
# survives GC and can be restored with a single command.
#
# Untracked files are deliberately left in place -- scratch inputs and local
# test directories are not what the quiz is gating.

set -euo pipefail

git_dir=$(git rev-parse --git-dir)
rm -f "$git_dir/claude-commit-quiz-pass"

stamp=$(date -u +%Y%m%d-%H%M%S)
branch="quiz-failed/$stamp"

# Capture tracked modifications alongside whatever was already staged.
git add -u

if git diff --cached --quiet; then
  echo "No tracked changes to park. Working tree already clean."
  exit 0
fi

tree=$(git write-tree)
parent=$(git rev-parse HEAD)

commit=$(git commit-tree "$tree" -p "$parent" -F - <<EOF
Parked by commit quiz gate ($stamp)

The user could not answer the comprehension questions for this change, so it
was not committed. Review the diff against $parent before reusing it.
EOF
)

git branch "$branch" "$commit"
git reset --hard HEAD

log="$git_dir/claude-commit-quiz.log"
echo "=== FAIL $(date -u +%Y-%m-%dT%H:%M:%SZ) parked=$branch commit=$commit" >> "$log"

cat <<EOF
Work parked on $branch (commit $commit). Tracked files reset to HEAD;
untracked files were left alone.

To bring it back:
    git cherry-pick -n $commit

To inspect it first:
    git show $commit
EOF
