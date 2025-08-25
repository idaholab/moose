#!/usr/bin/env python3
import os
import glob
import platform
import subprocess
import shutil
from setuptools import setup, Extension
from setuptools.command.build_ext import build_ext


THIS_DIR = os.path.dirname(__file__)
MOOSE_DIR = os.environ.get(
    "MOOSE_DIR", os.path.abspath(os.path.join(THIS_DIR, "..", "..", ".."))
)

# ---- Custom extension build with WASP ----------------------------------------


class BuildWithWasp(build_ext):
    def run(self):
        # Build or move wasp install
        wasp_include, wasp_lib, wasp_libraries = self._build_wasp()
        # Append include and lib stuff with wasp build
        ext: Extension  # For intellisense
        for ext in self.extensions:
            # This need to be relative paths
            ext.include_dirs.append(os.path.relpath(wasp_include, THIS_DIR))
            ext.library_dirs.append(os.path.relpath(wasp_lib, THIS_DIR))
            ext.libraries += wasp_libraries
            # Not sure if we need to also include $ORIGIN as well...
            ext.extra_link_args.append(f"-Wl,-rpath,{wasp_lib}")

        return super().run()

    def _build_wasp(self) -> tuple[str, str, list[str]]:
        # Make sure the source directory exists since we can't checkout the submodule
        wasp_srcdir = os.environ.get(
            "WASP_SRC_DIR", os.path.join(MOOSE_DIR, "framework", "contrib", "wasp")
        )
        if not os.path.isdir(wasp_srcdir) or not os.listdir(wasp_srcdir):
            raise RuntimeError(
                f"WASP submodule is not checked out in {wasp_srcdir}. Run either:\n"
                "    - ./scripts/update_and_rebuild_wasp.sh\n"
                f"   - git submodule update --init --recursive {wasp_srcdir}\n"
                f"In your MOOSE directory: {MOOSE_DIR}."
            )

        # Utilize the MOOSE update_and_rebuild_wasp script to build
        wasp_script = os.path.join(MOOSE_DIR, "scripts", "update_and_rebuild_wasp.sh")
        # Make sure the script exists
        if not os.path.exists(wasp_script):
            raise RuntimeError(
                f"Could not find WASP build script. Expected location:\n    {wasp_script}"
            )
        # Destination we will install to
        install_dir = os.path.abspath(os.path.join(self.build_lib, "moose_wasp"))
        # Need to copy over environment variables to the script
        env = os.environ.copy()
        # Install to our preferred location
        env["WASP_PREFIX"] = install_dir
        # This will throw if non-zero exit code
        subprocess.run(
            ["bash", wasp_script, "--skip-submodule-update"], env=env, check=True
        )

        # Directories from install
        incdir = os.path.join(install_dir, "include")
        libdir = os.path.join(install_dir, "lib")
        # Search for libraries
        ext = "dylib" if platform.system() == "Darwin" else "so"
        libs = []
        for file in glob.glob(os.path.join(libdir, f"lib*.{ext}")):
            name = os.path.basename(file).split(".", 1)[0][3:]
            libs.append(name)
        # If libraries were found, return data
        if os.path.isdir(incdir) and len(libs) > 0:
            # Return where everything is
            return incdir, libdir, libs
        # Otherwise the build somehow failed
        else:
            raise RuntimeError(f"WASP failed to build in {wasp_srcdir}")


# ---- Create extension --------------------------------------------------------

sources = [
    os.path.join("src", "hit", file)
    for file in ["parse.cc", "lex.cc", "braceexpr.cc", "hit.cpp"]
]

ext = Extension(
    name="hit",
    language="c++",
    sources=sources,
    include_dirs=["include"],  # WASP stuff filled by build_ext
    library_dirs=[],  # WASP stuff filled by build_ext
    libraries=[],  # WASP stuff filled by build_ext
    extra_compile_args=["-std=c++17", "-fPIC"],
    extra_link_args=(
        ["-undefined", "dynamic_lookup"]
        if platform.system().lower() == "darwin"
        else []
    ),  # WASP stuff filled by build_ext
)

# ---- Setup -------------------------------------------------------------------

setup(name="moose-hit", ext_modules=[ext], cmdclass={"build_ext": BuildWithWasp})
