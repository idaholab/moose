#!/usr/bin/env bash
#* This file is part of the MOOSE framework
#* https://mooseframework.inl.gov
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

# Set PIP_EXE to the pip command of the active Python environment.
function set_pip_exe()
{
  # Detect which pip is available. In a standard Python environment (e.g. conda)
  # `python3 -m pip` works; in the uv-created apptainer venv there is no pip module,
  # so fall back to the `pip` wrapper that forwards to `uv pip`. That wrapper sits
  # next to the venv's python3; a pip elsewhere on PATH would install into another
  # environment.
  if python3 -m pip --version &> /dev/null; then
    PIP_EXE=(python3 -m pip)
  elif command -v pip &> /dev/null; then
    local python_dir
    python_dir=$(dirname "$(python3 -c 'import sys; print(sys.executable)')") || return 1
    # -ef compares the directories themselves, so symlinked paths (e.g. /var and
    # /private/var on macOS) still match
    if ! [[ "$(dirname "$(command -v pip)")" -ef "$python_dir" ]]; then
      echo "ERROR: python3 has no pip module and the 'pip' on PATH belongs to another"
      echo "environment; refusing to install packages there."
      return 1
    fi
    PIP_EXE=(pip)
  else
    echo "ERROR: pip not found"
    return 1
  fi
}

# Build NEML2 from source and install it (non-editable) into the ACTIVE Python
# environment's site-packages -- the same environment that provides PyTorch. A
# single pip install yields BOTH the C++ artifacts MOOSE links against
# (libneml2*.so, neml2.pc, public headers) AND the Python package (e.g.
# neml2-compile, used by the cpp-aoti runtime).
#
# Separated so that it can be reused across NEML2 build scripts:
# - scripts/update_and_rebuild_neml2.sh
#
# Arguments:
#   1. Path to the NEML2 source directory
#   Remaining arguments are passed to pip verbatim (e.g.
#   --config-settings=cmake.define.FOO=BAR).
function pip_install_neml2()
{
  local src_dir="$1"
  shift
  # --no-build-isolation : the build must see the active env's torch (torch is a
  #                        runtime dep of NEML2, not a build requirement, so an
  #                        isolated build env would not have it).
  # --no-deps            : install NEML2 itself only. This leaves the environment's
  #                        dependencies -- most importantly the user's pinned torch
  #                        build -- untouched (a plain --force-reinstall would
  #                        otherwise re-resolve and clobber torch).
  # --force-reinstall
  # --no-cache-dir       : guarantee a genuine rebuild+reinstall even when the
  #                        source changed but the version string did not (pip's
  #                        wheel cache would otherwise serve a stale build).
  # NEML2_MPI=ON is the one build option MOOSE requires; any additional cmake
  # options are forwarded by the caller as further --config-settings arguments.
  set_pip_exe || exit 1

  "${PIP_EXE[@]}" install "$src_dir" \
    --no-build-isolation \
    --no-deps \
    --force-reinstall \
    --no-cache-dir \
    --config-settings=cmake.define.NEML2_MPI=ON \
    "$@"
}

# Install NEML2 Python dependencies from local source checkouts into the ACTIVE
# Python environment. These are the dependencies that conda-forge either does
# not ship (nmhit, pyzag) or ships only against its own pytorch (pyro-ppl),
# which would replace the PyTorch built from framework/contrib/pytorch.
#
# The packages are built without network access and without their
# dependencies, which the environment must already provide (moose-dev does). A
# package whose installed version matches its checkout and that imports is left
# untouched. Only the pip module can read the version of a checkout, so with the
# uv pip wrapper every checkout is reinstalled. A package whose checkout is
# missing is used as installed, e.g. when it was installed from PyPI instead; if
# it is not installed either, this fails.
#
# Arguments:
#   Any number of <distribution>:<module>:<checkout> triples
function pip_install_neml2_dependencies()
{
  set_pip_exe || return 1
  # uv pip does not support pip install --report, which reads checkout versions
  local pip_report=false
  [[ "${PIP_EXE[*]}" == "python3 -m pip" ]] && pip_report=true

  # Print the checkouts that need to be installed, one per line
  local plan
  plan=$(python3 -c '
import importlib, importlib.metadata as md, json, os, re, subprocess, sys

pip_report = sys.argv[1] == "true"
checkouts = {}
for arg in sys.argv[2:]:
    dist, module, path = arg.split(":", 2)
    if os.path.isfile(os.path.join(path, "pyproject.toml")):
        checkouts[path] = (dist, module)
        continue
    try:
        version = md.version(dist)
    except md.PackageNotFoundError:
        sys.exit(f"Error: {dist} is not installed and its source checkout {path} is missing.\n"
                 f"Initialize it (requires network access) with\n"
                 f"  git submodule update --init --checkout {path}")
    print(f"{dist}: no source checkout at {path}; using the installed {version}",
          file=sys.stderr)

if checkouts and not pip_report:
    for path, (dist, _) in checkouts.items():
        print(f"{dist}: the checkout version is unknown without the pip module; reinstalling",
              file=sys.stderr)
        print(path)
elif checkouts:
    # Read the name and version of each checkout from its build backend
    report = subprocess.run([sys.executable, "-m", "pip", "install", "--dry-run", "--quiet",
                             "--no-index", "--no-deps", "--no-build-isolation", "--report", "-",
                             *checkouts], stdout=subprocess.PIPE, text=True)
    if report.returncode != 0:
        sys.exit("Error: Failed to read the package metadata of " + " ".join(checkouts))
    # Distribution names compared in their normalized form (PEP 503)
    versions = {re.sub(r"[-_.]+", "-", item["metadata"]["name"]).lower():
                item["metadata"]["version"] for item in json.loads(report.stdout)["install"]}
    for path, (dist, module) in checkouts.items():
        version = versions.get(re.sub(r"[-_.]+", "-", dist).lower())
        if version is None:
            sys.exit(f"Error: {path} does not contain {dist}")
        try:
            installed = md.version(dist)
        except md.PackageNotFoundError:
            installed = None
        if installed == version:
            try:
                importlib.import_module(module)
                print(f"{dist}: {version} is already installed", file=sys.stderr)
                continue
            except Exception as e:
                print(f"{dist}: the installed {version} fails to import ({e}); reinstalling",
                      file=sys.stderr)
        elif installed is not None:
            print(f"{dist}: replacing the installed {installed} with {version}", file=sys.stderr)
        print(path)
' "$pip_report" "$@") || return
  [[ -z "$plan" ]] && return 0

  local install_dirs=() dir
  while IFS= read -r dir; do
    install_dirs+=("$dir")
  done <<< "$plan"

  # PIP_USER=0: never fall back to a user-site install outside the environment
  PIP_USER=0 "${PIP_EXE[@]}" install "${install_dirs[@]}" \
    --no-index \
    --no-deps \
    --no-build-isolation \
    --no-cache-dir
}

# Check that the Python environment satisfies the runtime dependencies of NEML2
# and, recursively, of those dependencies, because NEML2 and the packages from
# pip_install_neml2_dependencies are installed with --no-deps.
#
# Arguments:
#   1. Path to the NEML2 source directory
function check_neml2_python_dependencies()
{
  python3 -c '
import importlib.metadata as md, os, re, sys
try:
    import tomllib
except ImportError:
    # Python < 3.11; tomli is a dependency of scikit-build-core there
    import tomli as tomllib
try:
    from packaging.requirements import Requirement
except ImportError:
    from pip._vendor.packaging.requirements import Requirement

with open(os.path.join(sys.argv[1], "pyproject.toml"), "rb") as f:
    project = tomllib.load(f)["project"]

pending = [(project["name"], r) for r in project.get("dependencies", [])]
visited, problems = set(), []
while pending:
    parent, req = pending.pop()
    req = Requirement(req)
    # Extras are not installed, so requirements that only apply to an extra are skipped
    if req.marker and not req.marker.evaluate({"extra": ""}):
        continue
    try:
        version = md.version(req.name)
    except md.PackageNotFoundError:
        problems.append(f"{parent} requires {req}, which is not installed")
        continue
    if not req.specifier.contains(version, prereleases=True):
        problems.append(f"{parent} requires {req}, but {version} is installed")
    key = re.sub(r"[-_.]+", "-", req.name).lower()
    if key not in visited:
        visited.add(key)
        pending += [(req.name, r) for r in md.requires(req.name) or []]
if problems:
    print("Error: The active Python environment does not satisfy the dependencies of NEML2:",
          *problems, sep="\n  ", file=sys.stderr)
    sys.exit(1)
' "$1"
}
