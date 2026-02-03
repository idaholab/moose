# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

"""Build the MooseTest FMU and mark it as not requiring an execution tool."""

import os
import subprocess
import sys
import tempfile
import xml.etree.ElementTree as ET
import zipfile


def _set_needs_execution_tool(fmu_path: str) -> None:
    """Rewrite the FMU model description to set needsExecutionTool=false."""
    with zipfile.ZipFile(fmu_path, "r") as archive:
        try:
            xml_data = archive.read("modelDescription.xml")
        except KeyError as exc:
            raise RuntimeError("FMU is missing modelDescription.xml") from exc

        root = ET.fromstring(xml_data)
        co_simulation = root.find(".//CoSimulation")
        if co_simulation is None:
            raise RuntimeError("FMU modelDescription.xml missing CoSimulation element")

        co_simulation.set("needsExecutionTool", "false")

        updated_xml = ET.tostring(root, encoding="utf-8", xml_declaration=True)

        with tempfile.NamedTemporaryFile(
            dir=os.path.dirname(fmu_path),
            delete=False,
            suffix=".fmu",
        ) as tmp_file:
            tmp_path = tmp_file.name

        try:
            with zipfile.ZipFile(fmu_path, "r") as src, zipfile.ZipFile(
                tmp_path, "w", compression=zipfile.ZIP_DEFLATED
            ) as dst:
                for info in src.infolist():
                    if info.filename == "modelDescription.xml":
                        dst.writestr(info, updated_xml)
                    else:
                        dst.writestr(info, src.read(info.filename))
            os.replace(tmp_path, fmu_path)
        finally:
            if os.path.exists(tmp_path):
                os.unlink(tmp_path)


def main() -> int:
    """Build the FMU, patch modelDescription.xml, and return a process exit code."""
    result = subprocess.run(
        ["pythonfmu", "build", "-f", "MooseTest.py"], check=False
    )
    if result.returncode != 0:
        return result.returncode

    fmu_path = os.path.join(os.getcwd(), "MooseTest.fmu")
    if not os.path.exists(fmu_path):
        sys.stderr.write("MooseTest.fmu not found after pythonfmu build.\n")
        return 1

    _set_needs_execution_tool(fmu_path)
    print("Updated MooseTest.fmu: needsExecutionTool=false")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
