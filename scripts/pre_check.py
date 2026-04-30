#!/usr/bin/env python3
# * This file is part of the MOOSE framework
# * https://mooseframework.inl.gov
# *
# * All rights reserved, see COPYRIGHT for full restrictions
# * https://github.com/idaholab/moose/blob/master/COPYRIGHT
# *
# * Licensed under LGPL 2.1, please see LICENSE for details
# * https://www.gnu.org/licenses/lgpl-2.1.html

"""
Python port of env_pre_check.sh with an extended allow-list for Unicode characters.
- Mirrors the checks from the original shell script.
- Adds a configurable Unicode allow-list (ASCII + Latin Extended/European accents + Greek +
  Superscripts/Subscripts + ¹²³ + µ + math symbols).
"""

import argparse
import os
import re
import sys
import stat
import subprocess
import textwrap
import unicodedata
from pathlib import Path
from typing import Iterable

# --------------------------- Helpers ---------------------------


def _is_regular_git_file(mode: str) -> bool:
    """Return whether a git index mode represents a regular file."""
    return stat.S_ISREG(int(mode, 8))


def _is_executable_git_file(mode: str) -> bool:
    """Return whether a git index mode represents an executable file."""
    return bool(stat.S_IMODE(int(mode, 8)) & stat.S_IXUSR)


def _checkable_path(path: str) -> bool:
    """Return whether a tracked path should be included in precheck scans."""
    return (
        not path.startswith("contrib/")
        and "/contrib/" not in path
        and not path.endswith("/test_pre_check.py")
    )


def git_file_entries(*patterns: str) -> list[tuple[str, str]]:
    """Return (mode, path) for tracked regular files matching glob patterns."""
    try:
        cp = subprocess.run(
            ["git", "ls-files", "-z", "--stage", "--cached", "--", *patterns],
            capture_output=True,
            text=True,
            check=True,
        )
    except subprocess.CalledProcessError as e:
        print("Git error output:", e.stderr)
        raise

    entries = []
    for entry in cp.stdout.split("\x00"):
        if not entry:
            continue

        metadata, path = entry.split("\t", 1)
        mode = metadata.split(" ", 1)[0]
        if _is_regular_git_file(mode) and _checkable_path(path):
            entries.append((mode, path))

    return entries


def git_files(*patterns: str) -> list[str]:
    """Return tracked regular files matching glob patterns, excluding contrib/."""
    return [path for _, path in git_file_entries(*patterns)]


def read_text(path: str) -> str:
    """Read file as UTF-8 text, replacing invalid byte sequences."""
    with open(path, "r", encoding="utf-8", errors="replace") as f:
        return f.read()


# --------------------------- Ticket references ---------------------------


def ticket_references(log_from: str, log_to: str) -> str:
    r"""Find commit messages with ticket references (#1234, moose/issues/, moose/pull/)."""
    cp = subprocess.run(
        ["git", "log", f"{log_from}..{log_to}"],
        capture_output=True,
        text=True,
        check=True,
    )
    pat = re.compile(r"(?:moose/(?:issues|pull)/)|#\d+")
    return "\n".join(
        line for line in (cp.stdout or "").splitlines() if pat.search(line)
    )


# --------------------------- File set helpers ---------------------------


def files_for_tabs_except_input() -> list[str]:
    return git_files("*.[Ch]", "*.py")


def files_for_tabs() -> list[str]:
    return git_files("*.[Chi]", "*.py")


def files_for_whitespace(check_cpp: bool, check_fortran: bool) -> list[str]:
    """Get files to check for trailing whitespace."""
    files = ["*.[Cchi]", "*.py"] if check_cpp else ["*.i", "*.py"]
    if check_fortran:
        files += ["*.[FfH]", "*.f90", "*.F90", "*.FF90"]
    return git_files(*files)


def files_for_headers() -> list[str]:
    return git_files("*.h")


# --------------------------- Individual checks ---------------------------


def find_tabs(files: Iterable[str]) -> list[str]:
    """Return files containing tab characters."""
    return [f for f in files if "\t" in read_text(f)]


def banned_keywords() -> list[str]:
    r"""Return C/C++ files containing banned keywords (std::cout, std::cerr, printf, sleep, print_trace)."""
    bad = []
    for f in git_files("*.[Ch]"):
        text = read_text(f)
        if "std::cout" in text or "std::cerr" in text:
            bad.append(f)
        elif re.search(r"printf\s*\(", text):
            bad.append(f)
        elif not f.endswith("SlowProblem.C") and re.search(r"sleep\s*\(", text):
            bad.append(f)
        elif not re.search(r"MooseError\.(h|C)$", f) and "print_trace" in text:
            bad.append(f)
    return bad


def banned_funcs() -> list[str]:
    r"""Return files containing deprecated MOOSE function calls (mooseError2, mooseWarning2, etc.)."""
    pat_file_ok = re.compile(r"Moose(Error|Object)\.(h|C)$")
    pat = re.compile(
        r"moose(Warning|Error|Deprecated|Info)2\s*\(", re.IGNORECASE | re.DOTALL
    )
    return [
        f
        for f in git_files("*.[Ch]")
        if not pat_file_ok.search(f) and pat.search(read_text(f))
    ]


def classified_keywords() -> list[str]:
    r"""Return files containing classified or proprietary keywords."""
    pat_prop = re.compile(r"p\s*r\s*o\s*p\s*r\s*i\s*e\s*t\s*a\s*r\s*y", re.IGNORECASE)
    pat_class = re.compile(r"c\s*l\s*a\s*s\s*s\s*i\s*f\s*i\s*e\s*d", re.IGNORECASE)
    bad = []
    for f in git_files("*.[Chi]", "*.py"):
        if f.endswith("pre_check.py"):
            continue
        text = read_text(f)
        if pat_prop.search(text) or pat_class.search(text):
            bad.append(f)
    return bad


def trailing_whitespace_files(files: Iterable[str]) -> list[str]:
    """Return files containing trailing whitespace."""
    bad = []
    for f in files:
        with open(f, "r", encoding="utf-8", errors="replace") as fh:
            if any(re.search(r"\s+$", line.rstrip("\n")) for line in fh):
                bad.append(f)
    return bad


def no_newline_at_eof_files() -> list[str]:
    """Return files missing a newline at end of file."""
    bad = []
    for f in git_files("*.[Chi]", "*.py"):
        with open(f, "rb") as fh:
            fh.seek(0, 2)
            if fh.tell() == 0:
                continue
            fh.seek(-1, 2)
            if fh.read(1) != b"\n":
                bad.append(f)
    return bad


def find_bad_executables() -> list[str]:
    """Return files with incorrect executable permissions.

    Scripts (.py, .js, .sh, .pl) and extensionless files are allowed to be executable.
    """
    bad = []
    for mode, f in git_file_entries():
        if f.endswith((".pl", ".py", ".js", ".sh")) or "." not in Path(f).name:
            continue
        if _is_executable_git_file(mode):
            bad.append(f)
    return bad


def style_files() -> list[str]:
    r"""Return C/C++ files containing control keywords without a space before '('.

    Detects `if(`, `for(`, `while(`, `switch(` — keywords that should be
    written `if (`, `for (`, etc. per MOOSE coding standards.
    """
    pat = re.compile(r"\b(if|for|while|switch)\(")
    return [f for f in git_files("*.[Ch]") if pat.search(read_text(f))]


def include_guard_files() -> list[str]:
    """Return header files using old-style #ifndef/#define include guards."""
    pat = re.compile(r"^#ifndef\s+(\S+_H_?)\s*\n#define\s+\1", re.M)
    return [f for f in files_for_headers() if pat.search(read_text(f))]


def windows_line_endings() -> list[str]:
    """Return files with Windows-style (CRLF) line endings."""
    bad = []
    for f in git_files("*.[Chi]", "*.py", "*.md"):
        with open(f, "rb") as fh:
            if b"\r\n" in fh.read():
                bad.append(f)
    return bad


# --------------------------- Unicode allow-list check ---------------------------

# Notation/relation symbols used to describe math but not as inline operators
WIDER_EXTRAS = [
    "PLUS-MINUS SIGN",  # ±
    "MINUS-OR-PLUS SIGN",  # ∓
    "EQUALS SIGN",  # =
    "NOT EQUAL TO",  # ≠
    "ALMOST EQUAL TO",  # ≈
    "IDENTICAL TO",  # ≡
    "PROPORTIONAL TO",  # ∝
    "INFINITY",  # ∞
    "SQUARE ROOT",  # √
    "ELEMENT OF",  # ∈
    "NOT AN ELEMENT OF",  # ∉
    "CONTAINS AS MEMBER",  # ∋
    "DOES NOT CONTAIN AS MEMBER",  # ∌
    "DOUBLE-STRUCK CAPITAL R",  # ℝ
    "DOUBLE-STRUCK CAPITAL N",  # ℕ
    "DOUBLE-STRUCK CAPITAL Z",  # ℤ
    "DOUBLE-STRUCK CAPITAL Q",  # ℚ
    "DOUBLE-STRUCK CAPITAL C",  # ℂ
]

# Computational operators used directly in PDE/continuum mechanics expressions
CORE_NAMES = [
    # Dots / products
    "MIDDLE DOT",  # ·
    "BULLET OPERATOR",  # ∙
    "DOT OPERATOR",  # ⋅
    "RING OPERATOR",  # ∘
    # Calculus / vector
    "NABLA",  # ∇
    "INTEGRAL",  # ∫
    "DOUBLE INTEGRAL",  # ∬
    "TRIPLE INTEGRAL",  # ∭
    "CONTOUR INTEGRAL",  # ∮
    "SURFACE INTEGRAL",  # ∯
    "VOLUME INTEGRAL",  # ∰
    "PARTIAL DIFFERENTIAL",  # ∂
    # Product / cross / direct sum
    "MULTIPLICATION SIGN",  # ×
    "VECTOR OR CROSS PRODUCT",  # ⨯
    "CIRCLED TIMES",  # ⊗
    "N-ARY CIRCLED TIMES OPERATOR",  # ⨂
    "CIRCLED DOT OPERATOR",  # ⊙
    "CIRCLED PLUS",  # ⊕
]

LEGACY_SUPERSCRIPTS = [
    "SUPERSCRIPT ONE",
    "SUPERSCRIPT TWO",
    "SUPERSCRIPT THREE",
]  # ¹ ² ³
MICRO_SIGN = "MICRO SIGN"  # µ

ALLOWED_NAMED_CHARS = {
    unicodedata.lookup(n)
    for n in CORE_NAMES + WIDER_EXTRAS + LEGACY_SUPERSCRIPTS + [MICRO_SIGN]
}


def _in_greek(ch: str) -> bool:
    cp = ord(ch)
    # Greek and Coptic: U+0370 - U+03FF, Greek Extended: U+1F00 - U+1FFF
    return (0x0370 <= cp <= 0x03FF) or (0x1F00 <= cp <= 0x1FFF)


def _in_superscripts_subscripts(ch: str) -> bool:
    return 0x2070 <= ord(ch) <= 0x209F


def _in_latin_extended(ch: str) -> bool:
    """Latin-1 Supplement (U+00A0 - U+00FF) and Latin Extended-A/B (U+0100 - U+024F)."""
    return 0x00A0 <= ord(ch) <= 0x024F


def _is_allowed_manual(ch: str) -> bool:
    return (
        ord(ch) < 128
        or _in_latin_extended(ch)
        or _in_greek(ch)
        or _in_superscripts_subscripts(ch)
        or ch in ALLOWED_NAMED_CHARS
    )


def _disallowed_spans(text: str) -> list[tuple[int, str]]:
    """Return (index, char) pairs for characters not in the allow-list."""
    return [(i, ch) for i, ch in enumerate(text) if not _is_allowed_manual(ch)]


def _get_line_col(text: str, index: int) -> tuple[int, int]:
    """Convert a string index to 1-based (line, column)."""
    line = text.count("\n", 0, index) + 1
    col = index - text.rfind("\n", 0, index)
    return line, col


def unicode_files() -> tuple[list[str], list[str]]:
    """Return (bad_files, detailed_locations) for files containing disallowed Unicode."""
    bad = []
    locations = []
    for f in git_files("*.[Chi]", "*.py"):
        text = read_text(f)
        disallowed = _disallowed_spans(text)
        if disallowed:
            bad.append(f)
            for idx, ch in disallowed:
                line, col = _get_line_col(text, idx)
                char_info = f"U+{ord(ch):04X}"
                if ch.isprintable() and ch not in "\n\r\t":
                    char_info += f" '{ch}'"
                locations.append(f"\t  {f}:{line}:{col}: {char_info}")
    return bad, locations


# --------------------------- Main precheck logic ---------------------------


def _fmt_files(files: list[str]) -> str:
    return "\n".join(f"\t{f}" for f in files)


def precheck_errors(log_from: str, log_to: str) -> int:
    """Run all precheck validations and return 0 on pass, 1 on failure."""
    # --- Read toggles ---
    check_ticket = os.environ.get("CHECK_TICKET_REFERENCE", "1") == "1"
    check_keywords = os.environ.get("CHECK_KEYWORDS", "1") == "1"
    check_eof = os.environ.get("CHECK_EOF", "1") == "1"
    check_exes = os.environ.get("CHECK_EXECUTABLES", "0") == "1"
    check_whitespace = os.environ.get("CHECK_WHITESPACE", "1") == "1"
    check_tabs = os.environ.get("CHECK_TABS", "1") == "1"
    check_tabs_except_input = (
        os.environ.get("CHECK_TABS_EXCEPT_INPUT_FILES", "0") == "1"
    )
    check_classified = os.environ.get("CHECK_CLASSIFIED", "1") == "1"
    check_unicode = os.environ.get("CHECK_UNICODE", "0") == "1"
    check_include_guards = os.environ.get("CHECK_INCLUDE_GUARDS", "0") == "1"
    check_windows = os.environ.get("CHECK_WINDOWS_FILES", "1") == "1"
    check_cpp_whitespace = os.environ.get("CHECK_CPP_WHITESPACE", "0") == "1"
    check_f_whitespace = os.environ.get("CHECK_F_WHITESPACE", "0") == "1"
    check_banned_funcs = os.environ.get("CHECK_BANNED_FUNCS", "0") == "1"
    check_style        = os.environ.get("CHECK_STYLE",        "1") == "1"

    # --- Run checks ---
    ticket_reference = ""
    whitespace_bad: list[str] = []
    tab_bad: list[str] = []
    classified_bad: list[str] = []
    keyword_bad: list[str] = []
    style_bad: list[str] = []
    eof_bad: list[str] = []
    exe_bad: list[str] = []
    banned_func_bad: list[str] = []
    unicode_bad: list[str] = []
    unicode_locations: list[str] = []
    include_guard_bad: list[str] = []
    windows_bad: list[str] = []

    if check_ticket:
        ticket_reference = ticket_references(log_from, log_to)
        if ticket_reference and os.environ.get("VERBOSE", "0") == "1":
            print(f"TICKET_REFERENCES:\n{ticket_reference}")

    if check_whitespace:
        whitespace_bad = trailing_whitespace_files(
            files_for_whitespace(check_cpp_whitespace, check_f_whitespace)
        )
    if check_tabs:
        tab_bad = find_tabs(
            files_for_tabs_except_input()
            if check_tabs_except_input
            else files_for_tabs()
        )
    if check_classified:
        classified_bad = classified_keywords()
    if check_keywords:
        keyword_bad = banned_keywords()
    if check_style:
        style_bad = style_files()
    if check_eof:
        eof_bad = no_newline_at_eof_files()
    if check_exes:
        exe_bad = find_bad_executables()
    if check_banned_funcs:
        banned_func_bad = banned_funcs()
    if check_unicode:
        unicode_bad, unicode_locations = unicode_files()
    if check_include_guards:
        include_guard_bad = include_guard_files()
    if check_windows:
        windows_bad = windows_line_endings()

    # --- Tabulate results ---
    # Each entry: (enabled, files, pass_msg, disabled_msg, error_header, extra_error, locations)
    tabs_pass_msg = (
        "Your patch contains no tabs (input files not checked)."
        if check_tabs_except_input
        else "Your patch contains no tabs."
    )
    file_checks = [
        (
            check_whitespace,
            whitespace_bad,
            "Your patch contains no trailing whitespace.",
            "Whitespace check disabled.",
            "ERROR: The following files contain trailing whitespace after applying your patch:",
            'Run the "delete_trailing_whitespace.sh" script in your $MOOSE_DIR/scripts directory.',
            [],
        ),
        (
            check_classified,
            classified_bad,
            "Your patch contains no proprietary or classified keywords.",
            "Classified keyword check disabled.",
            "ERROR: The following files contain classified or proprietary keywords:",
            "",
            [],
        ),
        (
            check_tabs,
            tab_bad,
            tabs_pass_msg,
            "Tabs check disabled.",
            "ERROR: MOOSE prefers two spaces instead of tabs. The following files contain tab characters:",
            "",
            [],
        ),
        (
            check_eof,
            eof_bad,
            "Your patch contains no files without newlines before EOF.",
            "EOF check disabled",
            "ERROR: The following files do not contain a newline character before EOF:",
            'Run the "delete_trailing_whitespace.sh" script in your $MOOSE_DIR/scripts directory.',
            [],
        ),
        (
            check_exes,
            exe_bad,
            "Your patch contains no bad executable files.",
            "Executable file check disabled",
            "ERROR: The following files are executable but shouldn't be:",
            "",
            [],
        ),
        (
            check_banned_funcs,
            banned_func_bad,
            "Your patch contains no banned functions.",
            "Banned function check disabled",
            "ERROR: The following files contain banned functions (e.g. mooseError2, mooseWarning2, etc.)\n"
            "Use mooseError, mooseWarning, etc. instead:",
            "",
            [],
        ),
        (
            check_keywords,
            keyword_bad,
            "Your patch contains no banned keywords.",
            "Keywords check disabled",
            "ERROR: The following files contain banned keywords (std::cout, std::cerr, sleep, print_trace):",
            "",
            [],
        ),
        (
            check_style,
            style_bad,
            "Your patch contains proper spacing after control keywords.",
            "Style check disabled",
            "ERROR: The following files contain control keywords without proper spacing"
            " (if, for, while, or switch):",
            "",
            [],
        ),
        (
            check_unicode,
            unicode_bad,
            "Your patch contains no disallowed unicode characters.",
            "Unicode check disabled",
            "ERROR: The following files contain disallowed unicode characters:",
            "",
            unicode_locations,
        ),
        (
            check_include_guards,
            include_guard_bad,
            "Your patch contains no old style C++ include guards.",
            "Include Guard check disabled",
            'ERROR: The following files contain include guards, MOOSE uses "#pragma once":',
            "",
            [],
        ),
        (
            check_windows,
            windows_bad,
            "Your patch contains no windows line endings.",
            "Windows line ending check disabled",
            "ERROR: The following files contain windows line endings:",
            "",
            [],
        ),
    ]

    # --- Determine pass/fail ---
    ticket_failed = check_ticket and not ticket_reference
    failed = ticket_failed or any(
        enabled and files for enabled, files, *_ in file_checks
    )
    any_passed = (check_ticket and bool(ticket_reference)) or any(
        enabled and not files for enabled, files, *_ in file_checks
    )

    # --- Info block ---
    if any_passed:
        info_msgs = []
        if check_ticket:
            if ticket_reference:
                info_msgs.append("Your patch contains a valid ticket reference.")
        else:
            info_msgs.append("Ticket reference check disabled.")

        for enabled, files, pass_msg, disabled_msg, *_ in file_checks:
            if enabled:
                if not files:
                    info_msgs.append(pass_msg)
            else:
                info_msgs.append(disabled_msg)

        print("\n" + "#" * 74)
        for msg in info_msgs:
            print(f"INFO: {msg}")
        print("#" * 74 + "\n")

    # --- Error block ---
    if failed:
        error_output = "\n" + "#" * 74 + "\n"

        if ticket_failed:
            error_output += "ERROR: Your patch does not contain a valid ticket reference! (i.e. #1234)\n"
            cp = subprocess.run(
                ["git", "log", f"{log_from}..{log_to}", "--pretty=%s"],
                capture_output=True,
                text=True,
                check=True,
            )
            error_output += cp.stdout + "\n"

        for enabled, files, _, __, error_header, extra_error, locations in file_checks:
            if not (enabled and files):
                continue
            error_output += f"\n{error_header}\n{_fmt_files(files)}\n"
            if extra_error:
                error_output += f"\n{extra_error}\n"
            if locations:
                error_output += f"\nDetailed locations:\n" + "\n".join(locations) + "\n"

        error_output += "#" * 74 + "\n"
        print(error_output)

    return int(failed)


# --------------------------- Entry point ---------------------------


def main(argv: list[str]) -> int:
    """Entry point."""
    parser = argparse.ArgumentParser(
        description="Run pre-commit checks on changes relative to a base commit."
    )
    parser.add_argument(
        "--base",
        metavar="COMMIT",
        help=(
            "Base commit to compare against HEAD. "
            "When omitted, the merge-base with devel is used."
        ),
    )
    args = parser.parse_args(argv[1:])

    if args.base:
        log_from = args.base
    else:
        for ref in ("origin/devel", "devel"):
            cp = subprocess.run(
                ["git", "merge-base", ref, "HEAD"],
                capture_output=True, text=True,
            )
            if cp.returncode == 0:
                log_from = cp.stdout.strip()
                print(f"Using merge-base with {ref}: {log_from[:8]}..HEAD", file=sys.stderr)
                break
        else:
            print(
                textwrap.dedent("""\
                    ERROR: Could not determine merge-base with origin/devel or devel.

                    Specify a base commit explicitly:
                      pre_check.py --base HEAD~1
                      pre_check.py --base <commit-hash>

                    Or ensure a 'devel' branch exists locally or as origin/devel.
                    """),
                file=sys.stderr,
            )
            return 1

    return precheck_errors(log_from, "HEAD")


if __name__ == "__main__":
    sys.exit(main(sys.argv))
