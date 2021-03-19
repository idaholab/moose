#!/usr/bin/env bash

for i in "$@"
do
  shift
  if [[ "$i" == "-h" || "$i" == "--help" ]]; then
    help=1;
  fi

  if [ "$i" == "--skip-submodule-update" ]; then
    skip_sub_update=1;
  fi
done

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
echo $SCRIPT_DIR

# Display help
if [[ -n "$help" ]]; then
  cd $SCRIPT_DIR/..
  echo "Usage: $0 [-h | --help | --skip-submodule-update ]"
  echo
  echo "-h | --help              Display this message"
  echo "--skip-submodule-update  Do not update the moosetools submodule, use the current version"
  echo "*************************************************************************************"
  echo ""
fi

cd $SCRIPT_DIR/..

git_dir=`git rev-parse --show-cdup 2>/dev/null`
if [[ -z "$skip_sub_update" && $? == 0 && "x$git_dir" == "x" ]]; then
  git submodule update --init moosetools
  if [[ $? != 0 ]]; then
    echo "git submodule command failed, are your proxy settings correct?"
    exit 1
  fi
fi

cd $SCRIPT_DIR/../moosetools/contrib/hit
make hit
make hit.so
exit $?
