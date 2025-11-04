#!/bin/bash
set -e

for var in APPLICATION_DIR APPLICATION_NAME SOURCE_DIR DEST_DIR; do
  if [ -z "${!var}" ]; then
    echo "$var not set"
    exit 1
  fi
done

if [ ! -d "$SOURCE_DIR" ]; then
  echo "ERROR: Source directory ${SOURCE_DIR} does not exist"
  exit 1
fi
rm -rf "$DEST_DIR"
mkdir -p "$DEST_DIR"

# Gets the path of one relative to another; needed because
# realpath --relative-to doesn't exist on mac
function relpath() {
  python -c "import os.path; print(os.path.relpath('$1', '$2'))";
}

# Find symlinks in the source. Done separately like this so that
# we can support files with spaces from find
links=()
while IFS=  read -r -d $'\0' link; do
  links+=("$link")
done < <(find "$SOURCE_DIR" -type l -print0)

# The root of the repository; we don't allow for "unsafely" resolving
# any symlinks that are out of the repository
repo_root=$(cd "$APPLICATION_DIR"; git rev-parse --show-toplevel)

# Filter symlinks for those broken or out of the application
rsync_args=()
for link in "${links[@]}"; do
  skip=
  # Links that resolve within directories that do not exist will fail this,
  # which is fine for now (they already didn't work). Relaive links
  # that point within the same directory (when the source doesn't exist)
  # will work.
  if ! source="$(readlink -f "$link")"; then
    skip="link is broken"
  else
    source_relative_to_app="$(relpath "$source" "$repo_root")"
    if [[ "$source_relative_to_app" == ..* ]]; then
      skip="resolves outside of ${repo_root}"
    fi
  fi
  if [ -n "$skip" ]; then
    echo "WARNING: Skipping ${link}; ${skip}"
    link_relative_to_source="$(relpath "$link" "$SOURCE_DIR")"
    rsync_args+=("--exclude" "$link_relative_to_source")
  fi
done

# Rely on --copy-unsafe-links to copy the content that cannot be accessed
# in the new directory
rsync -aq --copy-unsafe-links "${rsync_args[@]}" "$SOURCE_DIR/" "$DEST_DIR"

# Setup the testroot
if [ -e "${APPLICATION_DIR}/testroot" ]; then
  cp -f "${APPLICATION_DIR}/testroot" "${DEST_DIR}/"
elif [ -e "${SOURCE_DIR}/testroot" ]; then
  cp -f "${SOURCE_DIR}/testroot" "${DEST_DIR}/"
else
  echo "app_name = ${APPLICATION_NAME}" > "${DEST_DIR}/testroot";
fi
