#!/usr/bin/env python3
# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

"""
Resolve CXX to an underlying host C++ compiler and invoke neml2-compile.

MOOSE build and Conda environments set CXX to an MPI compiler wrapper (e.g.
mpicxx), which breaks TorchInductor's compiler detection and precompiled-header
generation during AOTI compilation. Unwrapping CXX to the underlying host
compiler while keeping the active toolchain's compiler selection (Clang vs GCC)
allows AOTI compilation to succeed across GCC, Clang, and Conda environments.
"""

import os
import shlex
import shutil
import subprocess
import sys

LAUNCHERS = {"ccache", "sccache", "distcc"}


def _strip_launchers(tokens: list[str]) -> list[str]:
    while tokens and os.path.basename(tokens[0]) in LAUNCHERS:
        tokens = tokens[1:]
    return tokens


def _validate_compiler(compiler: str) -> str | None:
    path = shutil.which(compiler)
    if not path:
        return None
    try:
        proc = subprocess.run(
            [path, "--version"],
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL,
            timeout=5,
            check=False,
        )
    except OSError:
        return None
    return path if proc.returncode == 0 else None


def _unwrap_mpi_compiler(cxx_spec: str) -> str | None:
    tokens = _strip_launchers(shlex.split(cxx_spec))
    if not tokens:
        return None

    compiler = tokens[0]
    wrapper_selectors = [t for t in tokens[1:] if t.startswith(("-cc=", "-cxx="))]

    for _ in range(2):
        exe = shutil.which(compiler)
        if not exe:
            return None

        unwrapped = None
        for probe_flag in ("--showme:command", "-show", "-compile-info"):
            try:
                proc = subprocess.run(
                    [exe, *wrapper_selectors, probe_flag],
                    stdout=subprocess.PIPE,
                    stderr=subprocess.DEVNULL,
                    text=True,
                    timeout=5,
                    check=False,
                )
            except OSError:
                continue

            if proc.returncode != 0 or not proc.stdout.strip():
                continue

            first_line = proc.stdout.strip().splitlines()[0]
            show_tokens = _strip_launchers(shlex.split(first_line))
            for tok in show_tokens:
                if not tok.startswith("-"):
                    if _validate_compiler(tok):
                        unwrapped = tok
                    break
            if unwrapped:
                break

        wrapper_selectors = []
        if not unwrapped or unwrapped == compiler:
            break
        compiler = unwrapped

    return _validate_compiler(compiler)


def resolve_cxx() -> str:
    """Return a validated host C++ compiler from CXX, or from PATH if CXX is unset."""
    raw_cxx = os.environ.get("CXX", "").strip()
    if raw_cxx:
        resolved = _unwrap_mpi_compiler(raw_cxx)
        if resolved:
            return resolved
        # Falling back to another compiler could silently select a toolchain other
        # than the one that built MOOSE
        raise RuntimeError(
            f"CXX={raw_cxx!r} does not resolve to a working C++ compiler."
        )

    for fallback in ("c++", "g++", "clang++"):
        resolved = _validate_compiler(fallback)
        if resolved:
            return resolved

    raise RuntimeError(
        "CXX is not set and no working C++ compiler (c++, g++, clang++) is on PATH."
    )


def _uses_clang_config(compiler: str) -> bool:
    """Return whether the compiler is a Clang driver loading a default config file."""
    try:
        proc = subprocess.run(
            [compiler, "--version"],
            stdout=subprocess.PIPE,
            stderr=subprocess.DEVNULL,
            text=True,
            timeout=5,
            check=False,
        )
    except OSError:
        return False
    # Clang prints this line only when it loaded a config file; GCC never does
    return "Configuration file:" in proc.stdout


def find_neml2_compile() -> str:
    """Locate the neml2-compile executable in the active Python environment or PATH."""
    env_bin = os.path.dirname(os.path.abspath(sys.executable))
    candidate = os.path.join(env_bin, "neml2-compile")
    if os.path.isfile(candidate) and os.access(candidate, os.X_OK):
        return candidate
    found = shutil.which("neml2-compile")
    if found:
        return found
    raise FileNotFoundError("Unable to locate 'neml2-compile' on PATH.")


def main() -> int:
    """Resolve CXX and exec neml2-compile with forwarded command-line arguments."""
    try:
        host_cxx = resolve_cxx()
        neml2_compile = find_neml2_compile()
    except Exception as exc:
        print(f"ERROR: {exc}", file=sys.stderr)
        return 1

    env = os.environ.copy()
    env["CXX"] = host_cxx
    # conda-forge's Clang driver config (*.cfg next to the compiler) adds
    # link-only -Wl,... flags, which Clang also applies to TorchInductor's
    # -x c++-header precompiled-header compilation because it runs without -c.
    # Disable the config only for a compiler that reports loading one.
    if _uses_clang_config(host_cxx):
        env.setdefault("CLANG_NO_DEFAULT_CONFIG", "1")

    os.execvpe(neml2_compile, [neml2_compile, *sys.argv[1:]], env)


if __name__ == "__main__":
    sys.exit(main())
