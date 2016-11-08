#!/bin/bash
# git pre-commit hook that runs an clang-format stylecheck.
# Features:
#  - abort commit when commit does not comply with the style guidelines
#  - create a patch of the proposed style changes

##################################################################
# SETTINGS
# set path to clang-format binary
CLANG_FORMAT=$(which clang-format)

# remove any older patches from previous commits. Set to true or false.
DELETE_OLD_PATCHES=false

# only parse files with the extensions in FILE_EXTS. Set to true or false.
# if false every changed file in the commit will be parsed with clang-format.
# if true only files matching one of the extensions are parsed with clang-format.
PARSE_EXTS=true

# file types to parse. Only effective when PARSE_EXTS is true.
FILE_EXTS=".C .h"

# only format files in these directories
DIRS_WHITELIST="framework/src framework/include"

##################################################################
# There should be no need to change anything below this line.

patch_only=0
apply=0
add_to_commit=0
against_prev=0
while test $# != 0
do
    case "$1" in
    --patch-only) patch_only=1 ;; # true to only print out a patch of the differences - no other text
    --apply) apply=1 ;; # true to automatically apply fixes
    --add-to-commit) add_to_commit=1; apply=1;; # true to automatically apply and add format fixes to git index
    --against-prev) against_prev=1;; # true to get file-list to check from diff against prev commit (instead of index)
    *)  usage ;;
    esac
    shift
done

# Reference: http://stackoverflow.com/questions/1055671/how-can-i-get-the-behavior-of-gnus-readlink-f-on-a-mac
canonicalize_filename () {
    local target_file=$1
    local physical_directory=""
    local result=""

    # Need to restore the working directory after work.
    pushd `pwd` > /dev/null

    cd "$(dirname "$target_file")"
    target_file=`basename $target_file`

    # Iterate down a (possible) chain of symlinks
    while [ -L "$target_file" ]
    do
        target_file=$(readlink "$target_file")
        cd "$(dirname "$target_file")"
        target_file=$(basename "$target_file")
    done

    # Compute the canonicalized name by finding the physical path
    # for the directory we're in and appending the target file.
    physical_directory=`pwd -P`
    result="$physical_directory"/"$target_file"

    # restore the working directory after work.
    popd > /dev/null

    echo "$result"
}

# exit on error
set -e

# check whether the given file matches any of the set extensions
matches_extension() {
    local filename=$(basename "$1")
    local extension=".${filename##*.}"
    local ext

    for ext in $FILE_EXTS; do [[ "$ext" == "$extension" ]] && return 0; done

    return 1
}

matches_subdir() {
    for dir in $DIRS_WHITELIST; do
        if [[ -n $(echo "$1" | grep "$dir") ]]; then
            return 0
        fi
    done
    return 1
}

# necessary check for initial commit
if [[ "$against_prev" == "1" ]]; then
    against=HEAD~
elif git rev-parse --verify HEAD >/dev/null 2>&1 ; then
    against=HEAD
else
    # Initial commit: diff against an empty tree object
    against=4b825dc642cb6eb9a060e54bf8d69288fbee4904
fi

if [ ! -x "$CLANG_FORMAT" ] ; then
    printf "Error: clang-format executable not found.\n"
    exit 1
fi

# create a random filename to store our generated patch
prefix="pre-commit-clang-format"
suffix="$(date +%s)"
patch="/tmp/$prefix-$suffix.patch"

# clean up any older clang-format patches
$DELETE_OLD_PATCHES && rm -f /tmp/$prefix*.patch

# create one patch containing all changes to the files
git diff-index --cached --diff-filter=ACMR --name-only $against -- | while read file;
do
    # ignore file if we do check for file extensions and the file
    # does not match any of the extensions specified in $FILE_EXTS
    if $PARSE_EXTS && ! matches_extension "$file"; then
        continue;
    fi
    if ! matches_subdir "$file" ; then
      continue;
    fi

    # clang-format our sourcefile, create a patch with diff and append it to our $patch
    # The sed call is necessary to transform the patch from
    #    --- $file timestamp
    #    +++ - timestamp
    # to both lines working on the same file and having a a/ and b/ prefix.
    # Else it can not be applied with 'git apply'.
    clang_args="-style=file"
    if [[ "$apply" == "1" ]]; then
        "$CLANG_FORMAT" $clang_args -i "$file"
        if [[ "$add_to_commit" == "1" ]]; then
            git add "$file"
        fi
    else
        "$CLANG_FORMAT" $clang_args "$file" | \
            diff -u "$file" - | \
            sed -e "1s|--- |--- a/|" -e "2s|+++ -|+++ b/$file|" >> "$patch"
    fi
done

# if no patch has been generated all is ok, clean up the file stub and exit
if [[ ! -s "$patch"  && "$apply" == "0" ]] ; then
    if [[ "$patch_only" == "0" ]]; then
        printf "Changes in this commit comply with code style rules.\n"
    fi
    rm -f "$patch"
    exit 0
fi

if [[ "$apply" == "1" ]]; then
    printf "Style changes applied.\n"
    exit 0
elif [[ "$patch_only" == "1" ]]; then
    cat "$patch"
else
    # a patch has been created, notify the user and exit
    printf "\nThe following differences were found between the code to commit "
    printf "and the clang-format rules:\n\n"
    cat "$patch"

    printf "\nYou can apply these changes with:\n git apply $patch\n"
    printf "(may need to be called from the root directory of your repository)\n"
    printf "Aborting commit. Apply changes and commit again or skip checking with"
    printf " --no-verify (not recommended).\n"
fi

exit 1
