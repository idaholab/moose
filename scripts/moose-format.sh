#!/bin/bash
# OPTIONAL : TICKET_SHAS: 1 will enable using sha instead of branch names, used on branch push
# OPTIONAL : CHECK_TICKET_REFERENCE : 0 will disable the check
# OPTIONAL : CHECK_STYLE : 0 will disable the check
# OPTIONAL : CHECK_KEYWORDS : 0 will disable the keyword check
# OPTIONAL : CHECK_EOF: 0 will disable the EOF check
# OPTIONAL : CHECK_EXECUTABLES: 1 will enable the executable check
# OPTIONAL : CHECK_AUTOFORMAT: 0 will disable automatic code formatting

REPO_DIR="."

function ticket_references()
{
  git log "$1".."$2" | perl -ne 'print if m<(?:moose/(?:issues|pull)/)|#\d{1,}>'
}

function git_files()
{
  # We need to have this null terminated to take care of filenames with spaces. The
  # ending perl does this.
  git ls-files "$@" |grep -v "^contrib/"|grep -v "/contrib/"|perl -pne 's/\n/\0/'
}

function tab_files()
{
  git_files "*.[Chi]" "*.py" |xargs -0 perl -nle 'if ($ARGV ne $oldargv && /\t/) { print "\t$ARGV"; $oldargv=$ARGV }'
}

function banned_keywords()
{
  git_files "*.[Chi]" |xargs -0 perl -nle 'if ($ARGV ne $oldargv && (/std::cout|std::cerr/ || /sleep\s*\(/ || ($ARGV !~ /MooseError.h/ && /print_trace/))) { print "\t$ARGV"; $oldargv=$ARGV }'
}

function classified_keywords()
{
  git_files "*.[Chi]" "*.py" |xargs -0 perl -nle 'if ($ARGV ne $oldargv && (/p\s*r\s*o\s*p\s*r\s*i\s*e\s*t\s*a\s*r\s*y/i || /c\s*l\s*a\s*s\s*s\s*i\s*f\s*i\s*e\s*d/i )) { print "\t$ARGV"; $oldargv=$ARGV }'
}

function style_files()
{
  git_files "*.[Ch]" |xargs -0 perl -nle 'if ($ARGV ne $oldargv &&   (/\bfor\(/ || /\bif\(/ || /\bwhile\(/ || /\bswitch\(/)) { print "\t$ARGV"; $oldargv=$ARGV }'
}

function whitespace_files()
{
  git_files "*.[Chi]" "*.py" | xargs -0 perl -nle 'if ($ARGV ne $oldargv && /\s+$/) { print "\t$ARGV"; $oldargv=$ARGV }'
}

function find_bad_executables()
{
  # Ignore *.py, *.js, *.sh, '*.pl' files
  # We also ignore files that don't have any extension
  git ls-files |grep -v "\.pl$" |grep -v "\.py$" |grep -v "\.js$" |grep -v "\.sh$"|grep "\."|perl -pne 's/\n/\0/'|xargs -0 perl -e 'foreach $argnum (0 .. $#ARGV) { print "\t$ARGV[$argnum]\n" if -X "$ARGV[$argnum]"; }'
}

function no_newline_at_eof_files()
{
  while read -rd '' line; do
    if [ "$(tail -c1 "$line")" != "" ]; then
      printf "\t$line\n"
    fi
  done < <(git_files '*.[Chi]' '*.py')
}

function precheck_errors()
{
  local check_ticket=${CHECK_TICKET_REFERENCE:-"1"}
  local check_style=${CHECK_STYLE:-"1"}
  local check_eof=${CHECK_EOF:-"1"}
  local check_keywords=${CHECK_KEYWORDS:-"1"}
  local check_exes=${CHECK_EXECUTABLES:-"0"}
  local check_autofmt=${CHECK_AUTOFORMAT:-"1"}
  local fail=0
  
  local have_clang=0
  if [ -n "$(which clang-format)" ]; then
    have_clang=1
  fi

  if [[ "$check_ticket" == "1" && "$TICKET_SHAS" == "1" ]]; then
      log_from="$base_sha"
      log_to="$head_sha"
      TICKET_REFERENCE=$(ticket_references "$log_from" "$log_to")
      echo $TICKET_REFERENCE
      if [ -z "$TICKET_REFERENCE" ]; then
        printf "\nERROR: Your patch does not contain a valid ticket reference! (i.e. #1234)\n"
        git log "$log_from".."$log_to" --pretty='%s'
        fail=1
      fi
  fi

  local -r CLASSIFIED_FILES=$(classified_keywords)
  if [ -n "$CLASSIFIED_FILES" ]; then
    fail=1
    printf "\nERROR: The following files contain keywords classified or proprietary keywords:\n${CLASSIFIED_FILES}\n"
  fi

  if [ "$check_keywords" == "1" ]; then
    BANNED_KEYWORDS=$(banned_keywords)
    if [ -n "$BANNED_KEYWORDS" ]; then
      fail=1
      printf "\nERROR: The following files contain banned keywords (std::cout, std::cerr, sleep, print_trace):\n${BANNED_KEYWORDS}\n"
    fi
  fi

  if [ "$check_exes" == "1" ]; then
    EXE_FILES=$(find_bad_executables)
    if [ -n "$EXE_FILES" ]; then
      fail=1
      printf "\nERROR: The following files are executable but shouldn't be:\n${EXE_FILES}\n"
    fi
  fi

  if [ "$check_autofmt" == "1" ]; then
    $REPO_DIR/scripts/autofmt.sh
    if [ "$?" != "0" ]; then
      fail=1
    fi
  elif [[ "$have_clang" == "0" ]]; then

    echo "checking whitespace..."
    local -r WHITESPACE_FILES=$(whitespace_files)
    if [ -n "$WHITESPACE_FILES" ]; then
      fail=1
      printf "\nERROR: The following files contain trailing whitespace after applying your patch:\n${WHITESPACE_FILES}\n"
      printf "\nRun the \"delete_trailing_whitespace.sh\" script in your \$MOOSE_DIR/scripts directory.\n"
    fi
  
    echo "checking for tab characters..."
    local -r TAB_FILES=$(tab_files)
    if [ -n "$TAB_FILES" ]; then
      fail=1
      printf "\nERROR: MOOSE prefers two spaces instead of tabs. The following files contain tab characters:\n${TAB_FILES}\n"
    fi
  
    echo "checking spacing..."
    if [ "$check_style" == "1" ]; then
      STYLE_FILES=$(style_files)
      if [ -n "$STYLE_FILES" ]; then
        fail=1
        printf "\nERROR: The following files contain control keywords without proper spacing (for, if, while, or switch)\nhttp://mooseframework.org/wiki/CodeStandards:\n${STYLE_FILES}\n"
      fi
    fi
  
    echo "checking for EOF newlines..."
    if [ "$check_eof" == "1" ]; then
      EOF_FILES=$(no_newline_at_eof_files)
      if [ -n "$EOF_FILES" ]; then
        fail=1
        printf "\nERROR: The following files do not contain a newline character before EOF:\n${EOF_FILES}\n"
        printf "\nRun the \"delete_trailing_whitespace.sh\" script in your \$MOOSE_DIR/scripts directory.\n"
      fi
    fi

  fi

  return $fail
}

echo "Checking formatting..."
precheck_errors

exit $?
