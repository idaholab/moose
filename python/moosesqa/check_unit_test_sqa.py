#!/usr/bin/env python3
# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details

import collections
import os
import re

import mooseutils
import pyhit

GoogleTest = collections.namedtuple("GoogleTest", ["identifier", "filename", "line"])
Diagnostic = collections.namedtuple("Diagnostic", ["filename", "line", "message"])
_Metadata = collections.namedtuple(
    "_Metadata", ["identifier", "filename", "line", "source"]
)
_Wrapper = collections.namedtuple("_Wrapper", ["parameters", "tests"])

GOOGLETEST_MACROS = ("TYPED_TEST_P", "TYPED_TEST", "TEST_P", "TEST_F", "TEST")
UNIT_TEST_SOURCE_RE = re.compile(r"(?:^|.*/)unit/src/.*\.(?:C|K)$")
UNIT_TEST_METADATA_RE = re.compile(r"(?:^|.*/)unit/src/.*\.unit_tests$")
IDENTIFIER_RE = re.compile(r"^[A-Za-z_][A-Za-z0-9_]*$")


def _sanitize_cpp(text):
    """Replace C++ comments and string/character literals with whitespace."""
    out = list(text)
    size = len(text)
    i = 0

    def blank(begin, end):
        for j in range(begin, end):
            if out[j] != "\n":
                out[j] = " "

    while i < size:
        if text.startswith("//", i):
            end = text.find("\n", i + 2)
            end = size if end == -1 else end
            blank(i, end)
            i = end
        elif text.startswith("/*", i):
            end = text.find("*/", i + 2)
            end = size if end == -1 else end + 2
            blank(i, end)
            i = end
        else:
            raw_prefix = next(
                (
                    prefix
                    for prefix in ('u8R"', 'uR"', 'UR"', 'LR"', 'R"')
                    if text.startswith(prefix, i)
                ),
                None,
            )
            if raw_prefix is not None:
                delimiter_begin = i + len(raw_prefix)
                delimiter_end = text.find("(", delimiter_begin)
                if (
                    delimiter_end != -1
                    and delimiter_end - delimiter_begin <= 16
                    and not re.search(
                        r"[\s\\()]",
                        text[delimiter_begin:delimiter_end],
                    )
                ):
                    delimiter = text[delimiter_begin:delimiter_end]
                    marker = ")" + delimiter + '"'
                    end = text.find(marker, delimiter_end + 1)
                    end = size if end == -1 else end + len(marker)
                    blank(i, end)
                    i = end
                    continue

            if text[i] in ('"', "'"):
                quote = text[i]
                end = i + 1
                while end < size:
                    if text[end] == "\\":
                        end += 2
                    elif text[end] == quote:
                        end += 1
                        break
                    else:
                        end += 1
                blank(i, min(end, size))
                i = end
            else:
                i += 1

    return "".join(out)


def _split_arguments(text):
    """Split a sanitized macro argument list at top-level commas."""
    arguments = []
    begin = 0
    depths = {"(": 0, "[": 0, "{": 0}
    closing = {")": "(", "]": "[", "}": "{"}
    for i, character in enumerate(text):
        if character in depths:
            depths[character] += 1
        elif character in closing:
            depths[closing[character]] -= 1
        elif character == "," and not any(depths.values()):
            arguments.append(text[begin:i].strip())
            begin = i + 1
    arguments.append(text[begin:].strip())
    return arguments


def _find_calls(text, names):
    """Yield macro calls as (name, offset, arguments)."""
    pattern = re.compile(
        r"\b({})\s*\(".format("|".join(sorted(names, key=len, reverse=True)))
    )
    for match in pattern.finditer(text):
        open_parenthesis = text.find("(", match.start())
        depth = 1
        i = open_parenthesis + 1
        while i < len(text) and depth:
            if text[i] == "(":
                depth += 1
            elif text[i] == ")":
                depth -= 1
            i += 1
        if depth == 0:
            yield (
                match.group(1),
                match.start(),
                _split_arguments(text[open_parenthesis + 1 : i - 1]),
            )


def _get_wrappers(text):
    """Return test-generating macro definitions and their source spans."""
    wrappers = {}
    spans = []
    lines = text.splitlines(keepends=True)
    offsets = []
    offset = 0
    for line in lines:
        offsets.append(offset)
        offset += len(line)

    i = 0
    while i < len(lines):
        if re.match(r"^\s*#\s*define\b", lines[i]):
            begin_line = i
            while lines[i].rstrip().endswith("\\") and i + 1 < len(lines):
                i += 1
            segment = "".join(lines[begin_line : i + 1])
            definition = re.match(
                r"^\s*#\s*define\s+([A-Za-z_][A-Za-z0-9_]*)\s*\((.*?)\)",
                segment,
                flags=re.DOTALL,
            )
            if definition is not None:
                name = definition.group(1)
                parameters = [
                    parameter.strip()
                    for parameter in definition.group(2).split(",")
                    if parameter.strip()
                ]
                tests = []
                body = segment[definition.end() :]
                for _, _, arguments in _find_calls(body, GOOGLETEST_MACROS):
                    if len(arguments) >= 2:
                        tests.append((arguments[0], arguments[1]))
                if tests:
                    wrappers[name] = _Wrapper(parameters, tests)

            begin = offsets[begin_line]
            end = offsets[i] + len(lines[i])
            spans.append((begin, end))
        i += 1

    return wrappers, spans


def _resolve_wrapper_identifier(expression, values):
    """Resolve a literal or parameter used for a generated suite/test name."""
    parts = expression.split("##")
    resolved = "".join(values.get(part.strip(), part.strip()) for part in parts)
    return resolved if IDENTIFIER_RE.fullmatch(resolved) else None


def discover_google_tests(filename, content=None):
    """Discover logical GoogleTest declarations in a C++ source file."""
    if content is None:
        with open(filename, "r", encoding="utf-8") as source:
            content = source.read()

    sanitized = _sanitize_cpp(content)
    wrappers, definition_spans = _get_wrappers(sanitized)
    searchable = list(sanitized)
    for begin, end in definition_spans:
        for i in range(begin, end):
            if searchable[i] != "\n":
                searchable[i] = " "
    searchable = "".join(searchable)

    tests = []
    for name, offset, arguments in _find_calls(
        searchable, GOOGLETEST_MACROS + tuple(wrappers)
    ):
        line = searchable.count("\n", 0, offset) + 1
        if name in GOOGLETEST_MACROS:
            if (
                len(arguments) >= 2
                and IDENTIFIER_RE.fullmatch(arguments[0])
                and IDENTIFIER_RE.fullmatch(arguments[1])
            ):
                tests.append(
                    GoogleTest(
                        "{}.{}".format(arguments[0], arguments[1]), filename, line
                    )
                )
        else:
            wrapper = wrappers[name]
            values = dict(zip(wrapper.parameters, arguments))
            for suite_expression, test_expression in wrapper.tests:
                suite = _resolve_wrapper_identifier(suite_expression, values)
                test = _resolve_wrapper_identifier(test_expression, values)
                if suite is not None and test is not None:
                    tests.append(
                        GoogleTest("{}.{}".format(suite, test), filename, line)
                    )

    return tests


def _relative_files(root_dir, tracked_files):
    if tracked_files is None:
        tracked_files = mooseutils.git_ls_files(root_dir)

    files = []
    for filename in tracked_files:
        absolute = (
            filename if os.path.isabs(filename) else os.path.join(root_dir, filename)
        )
        relative = os.path.relpath(absolute, root_dir).replace(os.sep, "/")
        files.append((relative, absolute))
    return files


def _unit_directory(filename):
    return filename.rsplit("/src/", 1)[0]


def _node_line(node, parameter=None):
    try:
        line = node.line(parameter, None) if parameter is not None else node.line()
    except TypeError:
        line = node.line(parameter) if parameter is not None else 1
    return line if line is not None else 1


def _inherited_value(node, parameter, tests_node, include_tests):
    current = node
    while current is not None and current is not tests_node:
        value = current.get(parameter, None)
        if value is not None:
            return value
        current = current.parent
    return tests_node.get(parameter, None) if include_tests else None


def _metadata_leaves(node):
    if not node.children:
        yield node
    else:
        for child in node.children:
            yield from _metadata_leaves(child)


def _read_metadata(filename, relative, source):
    records = []
    diagnostics = []
    try:
        root = pyhit.load(filename)
    except Exception as error:
        return records, [
            Diagnostic(
                relative, 1, "unable to parse unit-test SQA metadata: {}".format(error)
            )
        ]

    if len(root.children) != 1 or root.children[0].name != "Tests":
        return records, [
            Diagnostic(
                relative, 1, "unit-test SQA metadata must contain one [Tests] block"
            )
        ]

    tests_node = root.children[0]
    for leaf in _metadata_leaves(tests_node):
        line = _node_line(leaf)
        identifier = leaf.get("unit_test", None)
        if identifier is None or not str(identifier).strip():
            diagnostics.append(
                Diagnostic(
                    relative, line, "GoogleTest metadata leaf has no 'unit_test'"
                )
            )
            continue

        identifier = str(identifier).strip()
        unit_test_line = _node_line(leaf, "unit_test")
        if not re.fullmatch(
            r"[A-Za-z_][A-Za-z0-9_]*\.[A-Za-z_][A-Za-z0-9_]*", identifier
        ):
            diagnostics.append(
                Diagnostic(
                    relative,
                    unit_test_line,
                    "invalid logical GoogleTest identifier '{}'".format(identifier),
                )
            )
            continue

        records.append(_Metadata(identifier, relative, unit_test_line, source))

        if leaf.get("type", None) != "GoogleTest":
            diagnostics.append(
                Diagnostic(
                    relative,
                    _node_line(leaf, "type"),
                    "'{}' must have type = GoogleTest".format(identifier),
                )
            )

        required_values = {
            "requirement": _inherited_value(
                leaf, "requirement", tests_node, include_tests=False
            ),
            "design": _inherited_value(leaf, "design", tests_node, include_tests=True),
            "issues": _inherited_value(leaf, "issues", tests_node, include_tests=True),
        }
        for parameter, value in required_values.items():
            if value is None or not str(value).strip():
                diagnostics.append(
                    Diagnostic(
                        relative,
                        line,
                        "'{}' has no '{}' metadata".format(identifier, parameter),
                    )
                )

        if leaf.parent is not tests_node:
            detail = leaf.get("detail", None)
            if detail is None or not str(detail).strip():
                diagnostics.append(
                    Diagnostic(
                        relative,
                        line,
                        "grouped GoogleTest '{}' has no 'detail'".format(identifier),
                    )
                )

    return records, diagnostics


def _manifest_lines(filename):
    lines = collections.defaultdict(lambda: 1)
    if not os.path.isfile(filename):
        return lines
    directory = None
    with open(filename, "r", encoding="utf-8") as stream:
        for number, text in enumerate(stream, 1):
            key_match = re.match(r"^([^ ].*):\s*$", text)
            if key_match is not None:
                directory = key_match.group(1)
                continue
            entry_match = re.match(r"\s*-\s+['\"]?([^'\"]+)['\"]?\s*$", text)
            if directory is not None and entry_match is not None:
                lines[(directory, entry_match.group(1).strip())] = number
    return lines


def _read_legacy_manifest(filename, root_dir):
    relative = os.path.relpath(filename, root_dir).replace(os.sep, "/")
    if not os.path.isfile(filename):
        # No manifest means no legacy exemptions, not an error: a directory with no
        # unit/src GoogleTests has nothing to check, and one with tests and no manifest
        # simply gets no grandfathering -- every test needs real SQA metadata.
        return {}, []

    try:
        data = mooseutils.yaml_load(filename) or {}
    except Exception as error:
        return {}, [
            Diagnostic(relative, 1, "unable to parse legacy manifest: {}".format(error))
        ]

    if not isinstance(data, dict):
        return {}, [Diagnostic(relative, 1, "legacy manifest must be a mapping")]

    diagnostics = []
    legacy = {}
    entry_lines = _manifest_lines(filename)
    directories = list(data)
    if directories != sorted(directories):
        diagnostics.append(
            Diagnostic(relative, 1, "legacy manifest directory keys are not sorted")
        )

    for directory, identifiers in data.items():
        if not isinstance(identifiers, list):
            diagnostics.append(
                Diagnostic(
                    relative,
                    1,
                    "legacy manifest entry '{}' must be a list".format(directory),
                )
            )
            continue
        if identifiers != sorted(identifiers):
            diagnostics.append(
                Diagnostic(
                    relative,
                    1,
                    "legacy tests for '{}' are not sorted".format(directory),
                )
            )
        for identifier in identifiers:
            key = (directory, identifier)
            if key in legacy:
                diagnostics.append(
                    Diagnostic(
                        relative,
                        entry_lines[key],
                        "duplicate legacy entry '{}:{}'".format(directory, identifier),
                    )
                )
            legacy[key] = entry_lines[key]

    return legacy, diagnostics


def is_moose_repository(root_dir):
    """
    Determine whether root_dir is the MOOSE repository itself, as opposed to a
    downstream application. Used to distinguish the root-level 'unit/' directory's
    'framework' naming/layout from a downstream app's own repository root.
    """
    return os.path.isdir(os.path.join(root_dir, "framework"))


def discover_unit_test_directories(root_dir, tracked_files=None):
    """
    Find every directory containing a unit/src subdirectory with GoogleTest sources.

    Output:
        Sorted list of (module_root, legacy_manifest) tuples, where module_root is the
        path (relative to root_dir) of the directory containing 'unit/src' (an empty
        string for the repository root itself), and legacy_manifest is the absolute path
        to that directory's legacy manifest.
    """
    root_dir = os.path.abspath(root_dir)
    repository_files = _relative_files(root_dir, tracked_files)

    module_roots = set()
    for relative, _ in repository_files:
        if UNIT_TEST_SOURCE_RE.fullmatch(relative):
            unit_directory = _unit_directory(relative)
            module_roots.add(
                "" if unit_directory == "unit" else unit_directory[: -len("/unit")]
            )

    # In the MOOSE repository itself, the root-level 'unit/' directory belongs to
    # 'framework', which keeps its own 'doc/' elsewhere -- so its legacy manifest lives
    # under framework/doc instead. Downstream apps have no 'framework/' directory: their
    # 'unit/' and 'doc/' are both siblings at the repository root, like any other module.
    is_moose_repo = is_moose_repository(root_dir)

    results = []
    for module_root in sorted(module_roots):
        if module_root == "" and is_moose_repo:
            legacy_manifest = os.path.join(
                root_dir, "framework", "doc", "legacy_unit_tests.yml"
            )
        else:
            legacy_manifest = os.path.join(
                root_dir, module_root, "doc", "legacy_unit_tests.yml"
            )
        results.append((module_root, legacy_manifest))
    return results


def check_unit_test_sqa(root_dir, legacy_manifest=None, tracked_files=None):
    """Check GoogleTest declarations against SQA metadata and the legacy manifest."""
    root_dir = os.path.abspath(root_dir)
    if legacy_manifest is None:
        legacy_manifest = os.path.join(root_dir, "doc", "legacy_unit_tests.yml")
    elif not os.path.isabs(legacy_manifest):
        legacy_manifest = os.path.join(root_dir, legacy_manifest)

    repository_files = _relative_files(root_dir, tracked_files)
    sources = {
        relative: absolute
        for relative, absolute in repository_files
        if UNIT_TEST_SOURCE_RE.fullmatch(relative)
    }
    metadata_files = {
        relative: absolute
        for relative, absolute in repository_files
        if UNIT_TEST_METADATA_RE.fullmatch(relative)
    }

    diagnostics = []
    discovered = collections.defaultdict(list)
    source_test_ids = {}
    for relative, absolute in sources.items():
        source_tests = discover_google_tests(absolute)
        source_test_ids[relative] = {test.identifier for test in source_tests}
        for test in source_tests:
            record = GoogleTest(test.identifier, relative, test.line)
            discovered[(_unit_directory(relative), record.identifier)].append(record)

    for records in discovered.values():
        if len(records) > 1:
            for record in records:
                diagnostics.append(
                    Diagnostic(
                        record.filename,
                        record.line,
                        "duplicate logical GoogleTest identifier '{}' in '{}'".format(
                            record.identifier, _unit_directory(record.filename)
                        ),
                    )
                )

    metadata = collections.defaultdict(list)
    for relative, absolute in metadata_files.items():
        source_base = relative[: -len(".unit_tests")]
        candidates = [
            source_base + extension
            for extension in (".C", ".K")
            if source_base + extension in sources
        ]
        if len(candidates) != 1:
            diagnostics.append(
                Diagnostic(
                    relative,
                    1,
                    "metadata file must have exactly one matching .C or .K source",
                )
            )
            continue

        records, errors = _read_metadata(absolute, relative, candidates[0])
        diagnostics.extend(errors)
        for record in records:
            metadata[(_unit_directory(record.source), record.identifier)].append(record)

            if record.identifier not in source_test_ids[record.source]:
                diagnostics.append(
                    Diagnostic(
                        record.filename,
                        record.line,
                        "stale SQA entry '{}' is not declared in '{}'".format(
                            record.identifier, record.source
                        ),
                    )
                )

    for records in metadata.values():
        if len(records) > 1:
            for record in records:
                diagnostics.append(
                    Diagnostic(
                        record.filename,
                        record.line,
                        "duplicate SQA mapping for '{}'".format(record.identifier),
                    )
                )

    legacy, legacy_errors = _read_legacy_manifest(legacy_manifest, root_dir)
    diagnostics.extend(legacy_errors)
    manifest_relative = os.path.relpath(legacy_manifest, root_dir).replace(os.sep, "/")

    for key, line in legacy.items():
        if key not in discovered:
            diagnostics.append(
                Diagnostic(
                    manifest_relative,
                    line,
                    "stale legacy entry '{}:{}'".format(key[0], key[1]),
                )
            )
        if key in metadata:
            message = (
                "'{}:{}' is present in both SQA metadata and the legacy manifest"
            ).format(key[0], key[1])
            diagnostics.append(
                Diagnostic(
                    manifest_relative,
                    line,
                    message,
                )
            )

    for key, records in discovered.items():
        if key not in metadata and key not in legacy:
            for record in records:
                diagnostics.append(
                    Diagnostic(
                        record.filename,
                        record.line,
                        "GoogleTest '{}' has no SQA metadata".format(record.identifier),
                    )
                )

    return sorted(
        diagnostics, key=lambda item: (item.filename, item.line, item.message)
    )
