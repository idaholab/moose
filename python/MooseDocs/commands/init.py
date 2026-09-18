# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html
import os
import re
import sys
import glob
import logging
import yaml
import mooseutils
import MooseDocs
from MooseDocs.common import exceptions

LOG = logging.getLogger(__name__)


def command_line_options(subparser, parent):
    """Command line options for 'init' command."""
    parser = subparser.add_parser(
        "init", parents=[parent], help="Initialize new repository to use MooseDocs."
    )

    parser.add_argument(
        "--config", default="config.yml", help="The configuration file."
    )

    init_subparser = parser.add_subparsers(
        dest="init_command", help="Initialization command(s)."
    )

    sqa_parser = init_subparser.add_parser("sqa", help="Initialize SQA documentation.")
    group = sqa_parser.add_mutually_exclusive_group(required=True)
    group.add_argument(
        "--app", type=str, help="Name of application to use in SQA template files."
    )
    group.add_argument(
        "--module", type=str, help="Name of MOOSE module to use in SQA template files."
    )
    sqa_parser.add_argument(
        "--category",
        default=None,
        type=str,
        help="Name of application to use in SQA configuration category name; if not provided it will be the lower case of the 'app' or 'module' option.",
    )


def _write_file(filename, content):
    """Helper for easy mock"""
    with open(filename, "w") as fid:
        fid.write(content)


def _read_file(filename):
    """Helper for easy mock"""
    with open(filename, "r") as fid:
        return fid.read()


def init_sqa_docs(app, is_module, category):
    """Adds markdown files that load MOOSE SQA template files."""

    # Create glob applicable to module or app
    if is_module:
        glb = glob.glob(
            os.path.join(
                MooseDocs.MOOSE_DIR,
                "framework",
                "doc",
                "content",
                "templates",
                "sqa",
                "module*.template",
            )
        )
    else:
        glb = glob.glob(
            os.path.join(
                MooseDocs.MOOSE_DIR,
                "framework",
                "doc",
                "content",
                "templates",
                "sqa",
                "app*.template",
            )
        )

    # Loop through MOOSE SQA template files
    for tname in glb:
        template_name = os.path.basename(tname)

        # Application SQA document filename
        if is_module:
            fname = "{}{}".format(category, template_name[6:-9])
        else:
            fname = "{}{}".format(category, template_name[3:-9])

        if fname.endswith("index.md"):
            fname = "index.md"

        filename = os.path.join(os.getcwd(), "content", "sqa", fname)

        # Create the directory, if needed
        os.makedirs(os.path.dirname(filename), exist_ok=True)

        # Write the content to the output file
        if is_module:
            content = "!template load file=sqa/{} module={} category={}".format(
                template_name, app, category
            )
        else:
            content = "!template load file=sqa/{} app={} category={}".format(
                template_name, app, category
            )

        _write_file(filename, content)


def init_sqa_config(app, is_module, category):
    """Creates default file to load from MooseDocs configuration."""
    if is_module:
        template = os.path.join(os.path.dirname(__file__), "sqa_module.yml.template")
    else:
        template = os.path.join(os.path.dirname(__file__), "sqa_app.yml.template")

    with open(template, "r") as fid:
        content = fid.read()

    repo = mooseutils.git_repo(MooseDocs.ROOT_DIR)
    content = mooseutils.apply_template_arguments(
        content, repo=repo, category=category, app=app
    )

    filename = os.path.join(os.getcwd(), "sqa_{}.yml".format(category))
    _write_file(filename, content)


def init_sqa_report(app, is_module, category):
    """Creates default SQA report configuration file."""
    if is_module:
        template = os.path.join(
            os.path.dirname(__file__), "sqa_reports_module.yml.template"
        )
    else:
        template = os.path.join(
            os.path.dirname(__file__), "sqa_reports_app.yml.template"
        )

    with open(template, "r") as fid:
        content = fid.read()

    content = mooseutils.apply_template_arguments(content, category=category, app=app)
    filename = os.path.join(os.getcwd(), "sqa_reports.yml")
    _write_file(filename, content)


def update_config_with_sqa(filename, app, category):
    """Modifies the config.yml to support SQA."""

    with open(filename, "r") as fid:
        config = mooseutils.yaml_load(filename, include=False)

    if "MooseDocs.extensions.sqa" not in config["Extensions"]:
        config["Extensions"]["MooseDocs.extensions.sqa"] = dict()

    if "MooseDocs.extensions.template" not in config["Extensions"]:
        config["Extensions"]["MooseDocs.extensions.template"] = dict(active=True)

    sqa = config["Extensions"]["MooseDocs.extensions.sqa"]
    sqa["active"] = True

    relpath = os.path.join(
        "${ROOT_DIR}", os.path.relpath(os.getcwd(), MooseDocs.ROOT_DIR)
    )
    sqa["reports"] = mooseutils.IncludeYamlFile(
        [os.path.join(relpath, "sqa_reports.yml")],
        MooseDocs.ROOT_DIR,
        filename,
        include=False,
    )
    sqa["categories"] = dict()
    sqa["categories"]["framework"] = mooseutils.IncludeYamlFile(
        ["${MOOSE_DIR}/framework/doc/sqa_framework.yml"],
        MooseDocs.ROOT_DIR,
        filename,
        include=False,
    )
    sqa["categories"][category] = mooseutils.IncludeYamlFile(
        [os.path.join(relpath, "sqa_{}.yml".format(category))],
        MooseDocs.ROOT_DIR,
        filename,
        include=False,
    )
    mooseutils.yaml_write(filename, config)


def _insert_sorted_entry(
    text, header, entry_indent, pinned_keys, category, new_entry_lines, blank_separated, trailing_blank
):
    """Inserts a new key: ... entry into a block of a hand-maintained yml file, keeping
    every entry not in 'pinned_keys' sorted alphabetically (case-insensitive) and leaving
    the rest of the file untouched. Returns (new_text, changed); changed is False (and
    new_text == text) if 'category' is already present, so the caller can skip writing."""
    lines = text.splitlines(keepends=True)
    header_idx = next(i for i, line in enumerate(lines) if line.rstrip("\n") == header)
    start_idx = header_idx + 1

    entries = []
    i = start_idx
    while i < len(lines):
        stripped = lines[i].rstrip("\n")
        if stripped == "":
            i += 1
            continue
        if not stripped.startswith(entry_indent):
            break
        # A line is a new entry (rather than a continuation of the previous one) only
        # when its indentation is exactly 'entry_indent', not deeper.
        if stripped[len(entry_indent) : len(entry_indent) + 1] not in (" ", "\t"):
            key = stripped[len(entry_indent) :].split(":", 1)[0].strip()
            entries.append([key, [lines[i]], i])
        else:
            entries[-1][1].append(lines[i])
        i += 1
    end_idx = i

    module_entries = [e for e in entries if e[0] not in pinned_keys]
    if any(key.lower() == category.lower() for key, _, _ in module_entries):
        return text, False

    # Keep the pinned (non-module) entries exactly as originally written, including
    # any inconsistent internal spacing (e.g. no blank line between 'create_diff_report'
    # and 'moose_test' in sqa_reports.yml), rather than resynthesizing their separators.
    pinned_end_idx = module_entries[0][2] if module_entries else end_idx
    pinned_text = "".join(lines[start_idx:pinned_end_idx])

    module_entries = [[key, entry_lines] for key, entry_lines, _ in module_entries]
    insert_pos = next(
        (idx for idx, (key, _) in enumerate(module_entries) if key.lower() > category.lower()),
        len(module_entries),
    )
    module_entries.insert(insert_pos, [category, new_entry_lines])

    block_lines = []
    for idx, (_, entry_lines) in enumerate(module_entries):
        block_lines.extend(entry_lines)
        if blank_separated and idx != len(module_entries) - 1:
            block_lines.append("\n")
    if trailing_blank:
        block_lines.append("\n")

    return (
        "".join(lines[:start_idx]) + pinned_text + "".join(block_lines) + "".join(lines[end_idx:])
    ), True


def update_module_sqa_registration(app, category):
    """Registers a new module's local SQA docs in MOOSE's top-level SQA aggregate files
    (modules/doc/config.yml and modules/doc/sqa_reports.yml), inserting each new entry
    in alphabetical order among the existing module entries."""
    app_class_name = app.replace(" ", "")

    config_filename = os.path.join(MooseDocs.MOOSE_DIR, "modules", "doc", "config.yml")
    text, changed = _insert_sorted_entry(
        _read_file(config_filename),
        header="        categories:",
        entry_indent="            ",
        pinned_keys={"framework", "python"},
        category=category,
        new_entry_lines=[
            "            {0}: !include modules/{0}/doc/sqa_{0}.yml\n".format(category)
        ],
        blank_separated=False,
        trailing_blank=True,
    )
    if changed:
        _write_file(config_filename, text)
    else:
        LOG.warning("Category '%s' already exists in %s, skipping.", category, config_filename)

    reports_filename = os.path.join(MooseDocs.MOOSE_DIR, "modules", "doc", "sqa_reports.yml")
    text = _read_file(reports_filename)
    text, applications_changed = _insert_sorted_entry(
        text,
        header="Applications:",
        entry_indent="    ",
        pinned_keys={"framework"},
        category=category,
        new_entry_lines=[
            "    {}:\n".format(category),
            "        exe_directory: modules/combined\n",
            "        app_types:\n",
            "            - {}App\n".format(app_class_name),
            "        content_directory: modules/{}/doc/content\n".format(category),
            "        unregister:\n",
            "            - framework/doc/unregister.yml\n",
            "        remove:\n",
            "            - framework/doc/remove.yml\n",
        ],
        blank_separated=True,
        trailing_blank=True,
    )
    if not applications_changed:
        LOG.warning(
            "Category '%s' already exists in the Applications section of %s, skipping.",
            category,
            reports_filename,
        )

    text, requirements_changed = _insert_sorted_entry(
        text,
        header="Requirements:",
        entry_indent="    ",
        pinned_keys={"create_diff_report", "moose_test", "stork", "tutorials", "scripts", "python", "combined"},
        category=category,
        new_entry_lines=[
            "    {}:\n".format(category),
            "        directories:\n",
            "            - modules/{}\n".format(category),
        ],
        blank_separated=True,
        trailing_blank=False,
    )
    if not requirements_changed:
        LOG.warning(
            "Category '%s' already exists in the Requirements section of %s, skipping.",
            category,
            reports_filename,
        )

    if applications_changed or requirements_changed:
        _write_file(reports_filename, text)


def main(options):
    errno = 0

    LOG.warning(
        "The 'init' command is designed to help get started. The resulting setup "
        "is not guaranteed to work and modifications are expected."
    )

    if options.init_command == "sqa":
        if options.app:
            is_module = False
            appname = options.app
            category = options.category or options.app.lower()
        elif options.module:
            is_module = True
            appname = options.module
            category = options.category or options.module.lower()

        init_sqa_config(appname, is_module, category)
        init_sqa_docs(appname, is_module, category)
        init_sqa_report(appname, is_module, category)
        update_config_with_sqa(options.config, appname, category)
        if is_module:
            update_module_sqa_registration(appname, category)
    else:
        LOG.error("The 'init' command requires a sub-command, see --help.")
        errno = 1

    return errno
