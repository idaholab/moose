#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://mooseframework.inl.gov
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

"""
Python port of env_pre_check.sh with an extended allow-list for Unicode characters.
- Mirrors the checks from the original shell script.
- Adds a configurable Unicode allow-list (ASCII + Latin Extended/European accents + Greek + Superscripts/Subscripts + ¹²³ + µ + math symbols).
- Uses standard `re` module with manual Unicode classification.
"""

import os
import re
import sys
import stat
import subprocess
from pathlib import Path
from typing import Iterable, List, Tuple, Optional

# --------------------------- Helpers ---------------------------

def run(cmd: List[str], capture=True, text=True, check=False) -> subprocess.CompletedProcess:
    return subprocess.run(cmd, capture_output=capture, text=text, check=check)

def git_files(*patterns: str) -> List[str]:
    """
    Return repo-tracked files matching glob patterns, excluding contrib/ and /contrib/,
    using NUL delimiters to be robust to spaces. Mirrors bash git_files() function.
    """
    if not patterns:
        patterns = ('',)
    cmd = ["git", "ls-files", "-z", *patterns]
    cp = run(cmd)
    if cp.returncode != 0:
        return []
    items = [p for p in cp.stdout.split("\x00") if p]
    items = [p for p in items if not (p.startswith("contrib/") or "/contrib/" in p)]
    return items

def read_text_bytes(path: str) -> bytes:
    try:
        with open(path, "rb") as f:
            return f.read()
    except Exception:
        return b""

def read_text(path: str) -> str:
    try:
        with open(path, "r", encoding="utf-8", errors="replace") as f:
            return f.read()
    except Exception:
        # Fallback to bytes decode with replacement
        return read_text_bytes(path).decode("utf-8", errors="replace")

# --------------------------- Ticket references ---------------------------

def ticket_references(log_from: str, log_to: str) -> str:
    r"""
    Mirrors: git log "$1".."$2" | perl -ne 'print if m<(?:moose/(?:issues|pull)/)|#\d{1,}>'
    """
    try:
        cp = run(["git", "log", f"{log_from}..{log_to}"], capture=True, text=True)
        text = cp.stdout or ""
    except Exception:
        text = ""
    pat = re.compile(r"(?:moose/(?:issues|pull)/)|#\d{1,}")
    out = []
    for line in text.splitlines():
        if pat.search(line):
            out.append(line)
    return "\n".join(out)

# --------------------------- File set helpers ---------------------------

def files_for_tabs_except_input() -> List[str]:
    return git_files("*.[Ch]", "*.py")

def files_for_tabs() -> List[str]:
    return git_files("*.[Chi]", "*.py")

def files_for_whitespace(check_cpp_whitespace: str, check_f_whitespace: str) -> List[str]:
    if check_cpp_whitespace == "0":
        files = ["*.i", "*.py"]
    else:
        files = ["*.[Cchi]", "*.py"]
    if check_f_whitespace == "1":
        files += ["*.[FfH]", "*.f90", "*.F90", "*.FF90"]
    return git_files(*files)

def files_for_headers() -> List[str]:
    return git_files("*.h")

# --------------------------- Individual checks ---------------------------

def find_tabs(files: Iterable[str]) -> List[str]:
    bad = []
    seen = set()
    for f in files:
        if f in seen:
            continue
        seen.add(f)
        data = read_text(f)
        if "\t" in data:
            bad.append("\t" + f)
    return bad

def banned_keywords() -> List[str]:
    r"""
    Mirrors:
    /std::cout|std::cerr/ ||
    /printf\s*\(/ ||
    ($ARGV !~ /SlowProblem.C/ && /sleep\s*\(/) ||
    ($ARGV !~ /MooseError.(h|C)/ && /print_trace/)
    """
    bad = []
    seen = set()
    files = git_files("*.[Chi]")
    for f in files:
        if f in seen:
            continue
        seen.add(f)
        text = read_text(f)
        if "std::cout" in text or "std::cerr" in text:
            bad.append("\t" + f)
            continue
        if re.search(r"printf\s*\(", text):
            bad.append("\t" + f)
            continue
        if not f.endswith("SlowProblem.C") and re.search(r"sleep\s*\(", text):
            bad.append("\t" + f)
            continue
        if not re.search(r"MooseError\.(h|C)$", f) and "print_trace" in text:
            bad.append("\t" + f)
            continue
    return bad

def banned_funcs() -> List[str]:
    r"""
    Mirrors: if ($ARGV !~ /Moose(Error|Object).(h|C)/ && /moose(Warning|Error|Deprecated|Info)2 *\(/igs )
    """
    bad = []
    seen = set()
    files = git_files("*.[Ch]")
    pat_file_ok = re.compile(r"Moose(Error|Object)\.(h|C)$")
    pat = re.compile(r"moose(Warning|Error|Deprecated|Info)2\s*\(", re.IGNORECASE | re.DOTALL)
    for f in files:
        if f in seen:
            continue
        seen.add(f)
        if pat_file_ok.search(f):
            continue
        text = read_text(f)
        if pat.search(text):
            bad.append("\t" + f)
    return bad

def classified_keywords() -> List[str]:
    r"""
    Mirrors: /p\s*r\s*o\s*p\s*r\s*i\s*e\s*t\s*a\s*r\s*y/i || /c\s*l\s*a\s*s\s*s\s*i\s*f\s*i\s*e\s*d/i
    """
    bad = []
    seen = set()
    files = git_files("*.[Chi]", "*.py")
    pat_prop = re.compile(r"p\s*r\s*o\s*p\s*r\s*i\s*e\s*t\s*a\s*r\s*y", re.IGNORECASE)
    pat_class = re.compile(r"c\s*l\s*a\s*s\s*s\s*i\s*f\s*i\s*e\s*d", re.IGNORECASE)
    for f in files:
        if f in seen:
            continue
        seen.add(f)
        # Skip env_pre_check.py itself since it needs to check for these keywords
        if f.endswith("pre_check.py"):
            continue
        text = read_text(f)
        if pat_prop.search(text) or pat_class.search(text):
            bad.append("\t" + f)
    return bad

def trailing_whitespace_files(files: Iterable[str]) -> List[str]:
    bad = []
    seen = set()
    for f in files:
        if f in seen:
            continue
        seen.add(f)
        found = False
        try:
            with open(f, "r", encoding="utf-8", errors="replace") as fh:
                for line in fh:
                    if re.search(r"\s+$", line.rstrip("\n")):
                        found = True
                        break
        except Exception:
            pass
        if found:
            bad.append("\t" + f)
    return bad

def no_newline_at_eof_files() -> List[str]:
    bad = []
    files = git_files("*.[Chi]", "*.py")
    for f in files:
        try:
            with open(f, "rb") as fh:
                fh.seek(0, 2)
                size = fh.tell()
                if size == 0:
                    continue
                fh.seek(-1, 2)
                last = fh.read(1)
                if last != b"\n":
                    bad.append("\t" + f)
        except Exception:
            continue
    return bad

def find_bad_executables() -> List[str]:
    """
    Ignore *.py, *.js, *.sh, *.pl and files without extension.
    Flag files that are executable (-X).
    """
    bad = []
    try:
        cp = run(["git", "ls-files"])
        files = cp.stdout.splitlines()
    except Exception:
        files = []
    for f in files:
        if f.endswith((".pl", ".py", ".js", ".sh")):
            continue
        if "." not in Path(f).name:
            # ignore files with no extension
            continue
        try:
            st = os.stat(f)
            if stat.S_ISREG(st.st_mode) and os.access(f, os.X_OK):
                bad.append("\t" + f)
        except Exception:
            continue
    return bad

def include_guard_files() -> List[str]:
    """
    Print headers that contain old-style include guards (#ifndef FOO_H / #define FOO_H).
    """
    bad = []
    files = files_for_headers()
    pat = re.compile(r"^#ifndef\s+(\S+_H_?)\s*\n#define\s+\1", re.M)
    for f in files:
        text = read_text(f)
        if pat.search(text):
            bad.append("\t" + f)
    return bad

def windows_line_endings() -> List[str]:
    bad = []
    files = git_files("*.[Chi]", "*.py")
    for f in files:
        data = read_text_bytes(f)
        if b"\r\n" in data:
            bad.append("\t" + f)
    return bad

# --------------------------- Unicode allow-list check ---------------------------

# Optional slightly wider variant additions (explicit operators)
WIDER_EXTRAS = [
    "PLUS-MINUS SIGN",               # ±
    "MINUS-OR-PLUS SIGN",            # ∓
    "EQUALS SIGN",                   # =
    "NOT EQUAL TO",                  # ≠
    "ALMOST EQUAL TO",               # ≈
    "IDENTICAL TO",                  # ≡
    "PROPORTIONAL TO",               # ∝
    "INFINITY",                      # ∞
    "SQUARE ROOT",                   # √
    "CIRCLED DOT OPERATOR",          # ⊙
    "CIRCLED PLUS",                  # ⊕
    "ELEMENT OF",                    # ∈
    "DOUBLE-STRUCK CAPITAL R",       # ℝ
    "DOUBLE-STRUCK CAPITAL N",       # ℕ
    "DOUBLE-STRUCK CAPITAL Z",       # ℤ
    "DOUBLE-STRUCK CAPITAL Q",       # ℚ
    "DOUBLE-STRUCK CAPITAL C",       # ℂ
    "NOT AN ELEMENT OF",             # ∉
    "CONTAINS AS MEMBER",            # ∋
    "DOES NOT CONTAIN AS MEMBER",    # ∌
]

CORE_NAMES = [
    # Dots / operators
    "MIDDLE DOT",                    # ·
    "BULLET OPERATOR",               # ∙
    "DOT OPERATOR",                  # ⋅
    "RING OPERATOR",                 # ∘
    # Calculus / vector
    "NABLA",                         # ∇
    "INTEGRAL",                      # ∫
    "DOUBLE INTEGRAL",               # ∬
    "TRIPLE INTEGRAL",               # ∭
    "CONTOUR INTEGRAL",              # ∮
    "SURFACE INTEGRAL",              # ∯
    "VOLUME INTEGRAL",               # ∰
    "PARTIAL DIFFERENTIAL",          # ∂
    # Product / cross
    "MULTIPLICATION SIGN",           # ×
    "VECTOR OR CROSS PRODUCT",       # ⨯
    "CIRCLED TIMES",                 # ⊗
    "N-ARY CIRCLED TIMES OPERATOR",  # ⨂
]

LEGACY_SUPERSCRIPTS = ["SUPERSCRIPT ONE", "SUPERSCRIPT TWO", "SUPERSCRIPT THREE"]  # ¹ ² ³
MICRO_SIGN = "MICRO SIGN"  # µ

def _named_chars(names: Iterable[str]) -> List[str]:
    out = []
    import unicodedata
    for n in names:
        try:
            out.append(unicodedata.lookup(n))
        except KeyError:
            pass
    return out

ALLOWED_NAMED_CHARS = _named_chars(CORE_NAMES + WIDER_EXTRAS + LEGACY_SUPERSCRIPTS + [MICRO_SIGN])


def _in_greek(ch: str) -> bool:
    cp = ord(ch)
    # Greek and Coptic: U+0370 - U+03FF, Greek Extended: U+1F00 - U+1FFF
    return (0x0370 <= cp <= 0x03FF) or (0x1F00 <= cp <= 0x1FFF)

def _in_superscripts_subscripts(ch: str) -> bool:
    cp = ord(ch)
    return 0x2070 <= cp <= 0x209F

def _is_ascii(ch: str) -> bool:
    return ord(ch) < 128

def _in_latin_extended(ch: str) -> bool:
    """Check if character is in Latin-1 Supplement or Latin Extended blocks."""
    cp = ord(ch)
    # Latin-1 Supplement: U+0080 - U+00FF (includes most Western European accents)
    # This covers French, Spanish, German, Italian, Portuguese, etc.
    if 0x00A0 <= cp <= 0x00FF:  # Skip control chars, start from non-breaking space
        return True
    # Latin Extended-A: U+0100 - U+017F (Central European, Baltic)
    # Covers Polish, Czech, Croatian, Hungarian, etc.
    if 0x0100 <= cp <= 0x017F:
        return True
    # Latin Extended-B: U+0180 - U+024F (less common, but still European)
    if 0x0180 <= cp <= 0x024F:
        return True
    # Latin Extended Additional: U+1E00 - U+1EFF (Vietnamese and other special Latin)
    # Uncomment if needed: if 0x1E00 <= cp <= 0x1EFF: return True
    # IPA Extensions: U+0250 - U+02AF (phonetic symbols)
    # Uncomment if needed: if 0x0250 <= cp <= 0x02AF: return True
    return False

def _is_allowed_manual(ch: str) -> bool:
    # ASCII
    if _is_ascii(ch):
        return True
    # Latin extended (European accents)
    if _in_latin_extended(ch):
        return True
    # Greek
    if _in_greek(ch):
        return True
    # Superscripts/Subscripts block
    if _in_superscripts_subscripts(ch):
        return True
    # Named math symbols and special characters
    if ch in ALLOWED_NAMED_CHARS:
        return True
    return False

def _disallowed_spans(text: str) -> List[Tuple[int, str]]:
    r"""
    Return list of (index, char) for characters NOT in the allow-list.
    Uses manual classification since standard re doesn't support Unicode properties.
    """
    out = []
    for i, ch in enumerate(text):
        if not _is_allowed_manual(ch):
            out.append((i, ch))
    return out

def _get_line_col(text: str, index: int) -> Tuple[int, int]:
    """Convert a string index to line and column numbers (1-based)."""
    line = 1
    col = 1
    for i, ch in enumerate(text):
        if i == index:
            return line, col
        if ch == '\n':
            line += 1
            col = 1
        else:
            col += 1
    return line, col

def unicode_files() -> Tuple[List[str], List[str]]:
    """Returns (bad_files, detailed_locations)."""
    bad = []
    locations = []
    files = git_files("*.[Chi]", "*.py")
    seen = set()
    for f in files:
        if f in seen:
            continue
        seen.add(f)
        text = read_text(f)
        disallowed = _disallowed_spans(text)
        if disallowed:
            bad.append("\t" + f)
            # Collect detailed location info for each disallowed character
            for idx, ch in disallowed:
                line, col = _get_line_col(text, idx)
                # Show the character code point for clarity
                char_info = f"U+{ord(ch):04X}"
                if ch.isprintable() and ch not in ['\n', '\r', '\t']:
                    char_info += f" '{ch}'"
                locations.append(f"\t  {f}:{line}:{col}: {char_info}")
    return bad, locations

# --------------------------- Main precheck logic ---------------------------

def precheck_errors(log_from: str, log_to: str) -> int:
    # Read toggles (match the shell script defaults)
    check_ticket = os.environ.get("CHECK_TICKET_REFERENCE", "1")
    check_keywords = os.environ.get("CHECK_KEYWORDS", "1")
    check_eof = os.environ.get("CHECK_EOF", "1")
    check_exes = os.environ.get("CHECK_EXECUTABLES", "0")
    check_whitespace = os.environ.get("CHECK_WHITESPACE", "1")
    check_tabs = os.environ.get("CHECK_TABS", "1")
    check_tabs_except_input_files = os.environ.get("CHECK_TABS_EXCEPT_INPUT_FILES", "0")
    check_classified = os.environ.get("CHECK_CLASSIFIED", "1")
    check_unicode = os.environ.get("CHECK_UNICODE", "0")
    check_include_guards = os.environ.get("CHECK_INCLUDE_GUARDS", "0")
    check_windows_files = os.environ.get("CHECK_WINDOWS_FILES", "1")
    check_cpp_whitespace = os.environ.get("CHECK_CPP_WHITESPACE", "0")
    check_f_whitespace = os.environ.get("CHECK_F_WHITESPACE", "0")

    TICKET_REFERENCE = ""
    BANNED_KEYWORDS = ""
    EOF_FILES = ""
    WHITESPACE_FILES = ""
    TAB_FILES = ""
    CLASSIFIED_FILES = ""
    UNICODE_FILES = ""
    UNICODE_LOCATIONS = ""
    INCLUDE_GUARD_FILES = ""
    WINDOWS_FILES = ""
    EXE_FILES = ""
    BAN_FUNC_FILES = ""

    if check_ticket == "1":
        TICKET_REFERENCE = ticket_references(log_from, log_to)
        print(f"TICKET_REFERNCES:\n{TICKET_REFERENCE}")

    if check_whitespace == "1":
        WHITESPACE_FILES = "\n".join(trailing_whitespace_files(files_for_whitespace(check_cpp_whitespace, check_f_whitespace)))

    if check_tabs == "1":
        if check_tabs_except_input_files == "1":
            TAB_FILES = "\n".join(find_tabs(files_for_tabs_except_input()))
        else:
            TAB_FILES = "\n".join(find_tabs(files_for_tabs()))

    if check_classified == "1":
        CLASSIFIED_FILES = "\n".join(classified_keywords())

    if check_keywords == "1":
        BANNED_KEYWORDS = "\n".join(banned_keywords())

    if check_eof == "1":
        EOF_FILES = "\n".join(no_newline_at_eof_files())

    if check_exes == "1":
        EXE_FILES = "\n".join(find_bad_executables())

    if os.environ.get("CHECK_BANNED_FUNCS", "0") == "1":
        BAN_FUNC_FILES = "\n".join(banned_funcs())

    if check_unicode == "1":
        files, locations = unicode_files()
        UNICODE_FILES = "\n".join(files)
        UNICODE_LOCATIONS = "\n".join(locations)

    if check_include_guards == "1":
        INCLUDE_GUARD_FILES = "\n".join(include_guard_files())

    if check_windows_files == "1":
        WINDOWS_FILES = "\n".join(windows_line_endings())

    one_failed = 0
    one_passed = 0

    def _pf(cond_pass: bool):
        nonlocal one_failed, one_passed
        if cond_pass:
            one_passed = 1
        else:
            one_failed = 1

    if check_ticket == "1":
        _pf(bool(TICKET_REFERENCE))

    if check_whitespace == "1":
        _pf(not WHITESPACE_FILES)

    if check_classified == "1":
        _pf(not CLASSIFIED_FILES)

    if check_tabs == "1":
        _pf(not TAB_FILES)

    if check_eof == "1":
        _pf(not EOF_FILES)

    if check_exes == "1":
        _pf(not EXE_FILES)

    if os.environ.get("CHECK_BANNED_FUNCS", "0") == "1":
        _pf(not BAN_FUNC_FILES)

    if check_keywords == "1":
        _pf(not BANNED_KEYWORDS)

    if check_ticket == "1":  # note: duplicated in original script; keeping parity
        _pf(bool(TICKET_REFERENCE))

    if check_unicode == "1":
        _pf(not UNICODE_FILES)

    if check_include_guards == "1":
        _pf(not INCLUDE_GUARD_FILES)

    if check_windows_files == "1":
        _pf(not WINDOWS_FILES)

    if one_passed == 1:
        print("\n\n##########################################################################")
        if check_ticket == "1":
            if TICKET_REFERENCE:
                print("INFO: Your patch contains a valid ticket reference.")
        else:
            print("INFO: Ticket reference check disabled.")

        if check_whitespace == "1":
            if not WHITESPACE_FILES:
                print("INFO: Your patch contains no trailing whitespace.")
        else:
            print("INFO: Whitespace check disabled.")

        if check_classified == "1":
            if not CLASSIFIED_FILES:
                print("INFO: Your patch contains no proprietary or classified keywords.")
        else:
            print("INFO: Classified keyword check disabled.")

        if check_tabs == "1":
            if not TAB_FILES:
                if check_tabs_except_input_files == "1":
                    print("INFO: Your patch contains no tabs (input files not checked).")
                else:
                    print("INFO: Your patch contains no tabs.")
        else:
            print("INFO: Tabs check disabled.")

        if check_eof == "1":
            if not EOF_FILES:
                print("INFO: Your patch contains no files without newlines before EOF.")
        else:
            print("INFO: EOF check disabled")

        if check_exes == "1":
            if not EXE_FILES:
                print("INFO: Your patch contains no bad executable files.")
        else:
            print("INFO: Executable file check disabled")

        if os.environ.get("CHECK_BANNED_FUNCS", "0") == "1":
            if not BAN_FUNC_FILES:
                print("INFO: Your patch contains no banned functions.")
        else:
            print("INFO: Banned function check disabled")

        if check_keywords == "1":
            if not BANNED_KEYWORDS:
                print("INFO: Your patch contains no banned keywords.")
        else:
            print("INFO: Keywords check disabled")

        if check_unicode == "1":
            if not UNICODE_FILES:
                print("INFO: Your patch contains no disallowed unicode characters.")
        else:
            print("INFO: Unicode check disabled")

        if check_include_guards == "1":
            if not INCLUDE_GUARD_FILES:
                print("INFO: Your patch contains no old style C++ include guards.")
        else:
            print("INFO: Include Guard check disabled")

        if check_windows_files == "1":
            if not WINDOWS_FILES:
                print("INFO: Your patch contains no windows line endings.")
        else:
            print("INFO: Windows line ending check disabled")

        print("##########################################################################\n")

    if one_failed == 1:
        print("\n##########################################################################")
        if check_ticket == "1" and not TICKET_REFERENCE:
            print("ERROR: Your patch does not contain a valid ticket reference! (i.e. #1234)")
            try:
                cp = run(["git", "log", f"{log_from}..{log_to}", "--pretty=%s"])
                print(cp.stdout)
            except Exception:
                pass

        if check_whitespace == "1" and WHITESPACE_FILES:
            print("\nERROR: The following files contain trailing whitespace after applying your patch:")
            print(WHITESPACE_FILES)
            print('\nRun the "delete_trailing_whitespace.sh" script in your $MOOSE_DIR/scripts directory.')

        if check_classified == "1" and CLASSIFIED_FILES:
            print("\nERROR: The following files contain classified or proprietary keywords:")
            print(CLASSIFIED_FILES)

        if check_tabs == "1" and TAB_FILES:
            print("\nERROR: MOOSE prefers two spaces instead of tabs. The following files contain tab characters:")
            print(TAB_FILES)

        if check_eof == "1" and EOF_FILES:
            print("\nERROR: The following files do not contain a newline character before EOF:")
            print(EOF_FILES)
            print('\nRun the "delete_trailing_whitespace.sh" script in your $MOOSE_DIR/scripts directory.')

        if check_exes == "1" and EXE_FILES:
            print("\nERROR: The following files are executable but shouldn't be:")
            print(EXE_FILES)

        if os.environ.get("CHECK_BANNED_FUNCS", "0") == "1" and BAN_FUNC_FILES:
            print("\nERROR: The following files contain banned functions (e.g. mooseError2, mooseWarning2, etc.) - use mooseError, mooseWarning, etc:")
            print(BAN_FUNC_FILES)

        if check_keywords == "1" and BANNED_KEYWORDS:
            print("\nERROR: The following files contain banned keywords (std::cout, std::cerr, sleep, print_trace):")
            print(BANNED_KEYWORDS)

        if check_unicode == "1" and UNICODE_FILES:
            print("\nERROR: The following files contain disallowed unicode characters:")
            print(UNICODE_FILES)
            if UNICODE_LOCATIONS:
                print("\nDetailed locations:")
                print(UNICODE_LOCATIONS)

        if check_include_guards == "1" and INCLUDE_GUARD_FILES:
            print('\nERROR: The following files contain include guards, MOOSE uses "#pragma once".')
            print(INCLUDE_GUARD_FILES)

        if check_windows_files == "1" and WINDOWS_FILES:
            print("\nERROR: The following files contain windows line endings:")
            print(WINDOWS_FILES)

        print("##########################################################################\n")

    return one_failed

# --------------------------- Entry point ---------------------------

def main(argv: List[str]) -> int:
    if len(argv) < 3:
        # Try to use merge-base with upstream/next as default
        try:
            cp = run(["git", "merge-base", "upstream/next", "HEAD"], capture=True, text=True, check=True)
            log_from = cp.stdout.strip()
            log_to = "HEAD"
            print(f"Using merge-base with upstream/next: {log_from[:8]}..{log_to}", file=sys.stderr)
        except subprocess.CalledProcessError:
            print("ERROR: Could not determine merge-base with upstream/next", file=sys.stderr)
            print("", file=sys.stderr)
            print("Please specify a comparison range explicitly:", file=sys.stderr)
            print("  pre_check.py HEAD~1 HEAD", file=sys.stderr)
            print("  pre_check.py upstream/next HEAD", file=sys.stderr)
            print("  pre_check.py <commit-hash> HEAD", file=sys.stderr)
            print("", file=sys.stderr)
            print("Or ensure 'upstream' remote is configured:", file=sys.stderr)
            print("  git remote add upstream https://github.com/idaholab/moose.git", file=sys.stderr)
            print("  git fetch upstream", file=sys.stderr)
            return 1
    else:
        log_from, log_to = argv[1], argv[2]
    return precheck_errors(log_from, log_to)

if __name__ == "__main__":
    sys.exit(main(sys.argv))
