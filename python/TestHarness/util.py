# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

import platform, os, re
import subprocess
from mooseutils import colorText
from collections import OrderedDict
from dataclasses import dataclass
import json
import threading
import typing
from typing import Optional
import time


## Run a command and return the output, or ERROR: + output if retcode != 0
def runCommand(cmd, force_mpi_command=False, **kwargs):
    # On Windows it is not allowed to close fds while redirecting output
    should_close = platform.system() != "Windows"
    if force_mpi_command:
        mpi_command = os.environ.get("MOOSE_MPI_COMMAND")
        if mpi_command is not None:
            cmd = f"{mpi_command} -n 1 {cmd}"
    p = subprocess.run(
        cmd,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        close_fds=should_close,
        shell=True,
        text=True,
        **kwargs,
    )
    output = p.stdout
    if p.returncode != 0:
        output = "ERROR: " + output
    return output


@dataclass(kw_only=True)
class FormatResultEntry:
    """
    Structure for a formatted result to be printed in formatResult()
    """

    # Name of the entry
    name: str
    # Timing for the entry, optional
    timing: Optional[float] = None
    # Memory for the entry, optional
    memory: Optional[int] = None
    # JointStatus object for the entry, optional
    joint_status: Optional[str] = None
    # Detailed status message for the entry, optional
    status_message: Optional[str] = None
    # Caveats for the entry, optional
    caveats: Optional[list[str]] = None
    # Coloring for the caveats, optional
    caveat_color: Optional[str] = None


def formatResult(
    entry: FormatResultEntry,
    options,
    timing: Optional[bool] = None,
    memory: Optional[bool] = None,
) -> str:
    """
    Helper for prenting a one-line result for something.

    The entry will not be colored if options.colored = false.
    """
    # Support only one instance of a format identifier, but obey the order
    term_format = [
        str(v) for v in list(OrderedDict.fromkeys(list(options.term_format)))
    ]
    # container for every printable item (message and color)
    result = dict.fromkeys(term_format, (None, None))

    # Helper for adding a formatted entry
    def add(key: str, message: str, color: str = None) -> None:
        assert key in result
        if message:
            if key.isupper():
                message = message.upper()
            result[key] = (message, color)

    # Populate formatted for those we support, with requested items
    # specified by the user
    caveat_key, justification_key = None, None  # filled separately
    for key in term_format:
        key_lower, message, color = key.lower(), None, None

        # Store caveat for later
        if key_lower == "c":
            caveat_key = key
        # Store justification for later
        elif key_lower == "j":
            justification_key = key
        # Pre status (not the message, the status type)
        elif key_lower == "p" and entry.joint_status is not None:
            message, color = (
                entry.joint_status.status.rjust(8, " "),
                entry.joint_status.color,
            )
        # Status message; only print if the pre status above is not in the result
        # or is in the result and the message above is not the same as this one
        elif (
            key_lower == "s"
            and entry.status_message is not None
            and (
                "p" not in term_format
                or entry.joint_status is not None
                and entry.status_message != entry.joint_status.status
            )
        ):
            message = entry.status_message
            color = entry.joint_status.color if entry.joint_status else None
        # Name
        elif key_lower == "n":
            message = entry.name
        # Time; adjust the precision of time, so we can justify the length.
        # The higher the seconds, the lower the decimal point, ie:
        # [0.000s] - [100.0s]. Max: [99999s]
        elif (
            key_lower == "t" and entry.timing is not None and (options.timing or timing)
        ):
            actual = float(entry.timing)
            int_len = len(str(int(actual)))
            precision = min(3, max(0, (4 - int_len)))
            message = "[" + "{0: <6}".format("%0.*fs" % (precision, actual)) + "]"
        # Memory; skipped for now, see #32243
        # elif (
        #     key_lower == "m"
        #     and entry.timing is not None
        #     and options.timing
        #     and memory is not False
        # ):
        #     if entry.memory is not None or entry.timing == 0:
        #         value = int(entry.memory * 1e-6) if entry.memory else 0
        #         message = f"[{value:>4}MB]"
        #     else:
        #         message = "[   ?MB]"

        add(key, message, color)

    # Helper for the length of the result string so far, removing any color
    def len_result() -> int:
        strip = lambda v: re.sub(r"\033\[\d+m", "", v)
        return len(
            " ".join([strip(message) for message, _ in result.values() if message])
        )

    # Decorate Caveats
    if entry.caveats and caveat_key is not None:
        caveats = ",".join(entry.caveats)
        f_caveats = "[" + caveats + "]"  # +1 space created later by join
        character_count = len_result() + len(f_caveats) + 1

        # If caveats are the last items the user wants printed, allow caveats to
        # consume available character count beyond options.term_cols.
        # Else, we trim caveats:
        if term_format[-1] != "c" and character_count > options.term_cols:
            over_by_amount = character_count - options.term_cols
            f_caveats = "[" + caveats[: len(caveats) - (over_by_amount + 3)] + "...]"

        add(caveat_key, f_caveats, entry.caveat_color)

    # Fill the available space left, with dots
    if justification_key is not None:
        character_count = len_result() + 1  # +1 space created later by join
        j_dot = "." * max(0, (options.term_cols - character_count))
        add(justification_key, j_dot, "GREY")

    # Accumulate values
    values = []
    for message, color in result.values():
        if message:
            if color and options.colored:
                message = colorText(message, color)
            values.append(message)
    return " ".join(values)


def formatJobResult(
    job,
    options,
    status_message: bool = True,
    timing: Optional[bool] = None,
    memory: Optional[bool] = None,
    memory_per_proc: bool = False,
    caveats: bool = False,
) -> str:
    name = job.getTestName()
    joint_status = job.getJointStatus()

    # Determine status message if requested
    if status_message:
        status_message = (
            joint_status.message if joint_status.message else joint_status.status
        )

        # Format failed messages
        if job.isFail():
            status_message = f"FAILED ({status_message})"
    # Otherwise, just use the status itself
    else:
        status_message = joint_status.status

    # Color first directory if appropriate
    if options.color_first_directory and options.colored:
        first_directory = job.specs["first_directory"]
        prefix = colorText(first_directory, "CYAN")
        suffix = name.replace(first_directory, "", 1)
        name = prefix + suffix

    memory = job.getMaxMemory()
    if memory_per_proc and memory is not None:
        memory = int(memory / job.getTester().getProcs(options))
    entry = FormatResultEntry(
        name=name,
        timing=job.getTiming(),
        memory=memory,
        joint_status=job.getJointStatus(),
        status_message=status_message,
        caveats=job.getCaveats() if caveats else [],
        caveat_color=joint_status.color if job.isFail() else "CYAN",
    )
    return formatResult(entry, options, timing=timing, memory=memory)


## Color the error messages if the options permit, also do not color in bitten scripts because
# it messes up the trac output.
# supports weirded html for more advanced coloring schemes. \verbatim<r>,<g>,<y>,<b>\endverbatim All colors are bolded.


def getPlatform():
    raw_uname = platform.uname()
    if raw_uname[0].upper() == "DARWIN":
        return "darwin"
    return raw_uname[0].lower()


def getMachine():
    return platform.machine().lower()


def outputHeader(header, ending=True):
    """
    Returns text for output with a visual separator, i.e.:
    ##############################...
    <header>
    ##############################...
    """
    begin_sep = "#" * 80
    end_sep = f"{begin_sep}\n" if ending else ""
    return f"{begin_sep}\n{header}\n{end_sep}"


def getSharedOption(libmesh_dir):
    # Some tests may only run properly with shared libraries on/off
    # We need to detect this condition
    libtool = os.path.join(libmesh_dir, "contrib", "bin", "libtool")
    with open(libtool, "r") as f:
        for line in f:
            try:
                key, value = line.rstrip().split("=", 2)
            except Exception:
                continue

            if key == "build_libtool_libs":
                if value == "yes":
                    return "dynamic"
                if value == "no":
                    return "static"

    # Neither no nor yes?  Not possible!
    print("Error! Could not determine whether shared libraries were built.")
    exit(1)


def getInitializedSubmodules(root_dir):
    """
    Gets a list of initialized submodules.
    Input:
      root_dir[str]: path to execute the git command. This should be the root
        directory of the app so that the submodule names are correct
    Return:
      list[str]: List of iniitalized submodule names or an empty list if there was an error.
    """
    output = str(runCommand("git submodule status", cwd=root_dir))
    if output.startswith("ERROR"):
        return []
    # This ignores submodules that have a '-' at the beginning which means they are not initialized
    return re.findall(r"^[ +]\S+ (\S+)", output, flags=re.MULTILINE)


def parseMOOSEJSON(output: str, context: str) -> dict:
    try:
        output = output.split("**START JSON DATA**\n")[1]
        output = output.split("**END JSON DATA**\n")[0]
        return json.loads(output)
    except IndexError:
        raise Exception(f"Failed to find JSON header and footer from {context}")
    except json.decoder.JSONDecodeError:
        raise Exception(f"Failed to parse JSON from {context}")


def checkOutputForPattern(output, re_pattern):
    """
    Returns boolean of pattern match
    """
    if re.search(re_pattern, output, re.MULTILINE | re.DOTALL) == None:
        return False
    else:
        return True


def checkOutputForLiteral(output, literal):
    """
    Returns boolean of literal match
    """
    if output.find(literal) == -1:
        return False
    else:
        return True


def deleteFilesAndFolders(test_dir, paths, delete_folders=True):
    """
    Delete specified files

    test_dir:       The base test directory
    paths:          A list contianing files to delete
    delete_folders: Attempt to delete any folders created
    """
    for file in paths:
        full_path = os.path.join(test_dir, file)
        if os.path.exists(full_path):
            try:
                os.remove(full_path)
            except:
                print(("Unable to remove file: " + full_path))

    # Now try to delete directories that might have been created
    if delete_folders:
        for file in paths:
            path = os.path.dirname(file)
            while path != "":
                path, tail = os.path.split(path)
                try:
                    os.rmdir(os.path.join(test_dir, path, tail))
                except:
                    # There could definitely be problems with removing the directory
                    # because it might be non-empty due to checkpoint files or other
                    # files being created on different operating systems. We just
                    # don't care for the most part and we don't want to error out.
                    # As long as our test boxes clean before each test, we'll notice
                    # the case where these files aren't being generated for a
                    # particular run.
                    #
                    # TL;DR; Just pass...
                    pass


def trimOutput(output, max_size=None):
    """Trims the output given some max size"""
    if not max_size or len(output) < max_size or not output:
        return output

    first_part = int(max_size * (2.0 / 3.0))
    second_part = int(max_size * (1.0 / 3.0))
    trimmed = f"{output[:first_part]}"
    if trimmed[-1] != "\n":
        trimmed += "\n"
    sep = "#" * 80
    trimmed += f"\n{sep}\nOutput trimmed\n{sep}\n{output[-second_part:]}"
    return trimmed


class ScopedTimer:
    """
    Helper class that will print out a message if a certain amount
    of time has passed
    """

    def __init__(self, timeout: typing.Union[int, float], message: str):
        self.timeout = timeout
        self.message = message
        self._stop_event = threading.Event()
        self._printed = False

    def _check_timeout(self):
        if not self._stop_event.wait(self.timeout):
            print(self.message + "...", end="", flush=True)
            self._printed = True

    def __enter__(self):
        self._thread = threading.Thread(target=self._check_timeout)
        self._thread.start()
        self.start_time = time.time()

    def __exit__(self, exc_type, exc_val, exc_tb):
        self._stop_event.set()
        self._thread.join()
        if self._printed:
            elapsed_time = time.time() - self.start_time
            print(f" {elapsed_time:.2f} seconds", flush=True)
