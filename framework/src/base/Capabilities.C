//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// Needs to be included before Capabilities.h, which
// includes nlohmann/json_fwd.h
#include "nlohmann/json.h"

#include "Capabilities.h"
#include "CapabilityException.h"
#include "MooseUtils.h"
#include "Conversion.h"

#include <vector>

#include "libmesh/libmesh_config.h"
#include "petscconf.h"
#include "petscpkg_version.h"

#ifdef MOOSE_LIBTORCH_ENABLED
#include <torch/version.h>
#endif

#if __has_include(<torch/xpu.h>)
#include <torch/xpu.h>
#define MOOSE_HAVE_XPU 1
#endif

#ifdef MOOSE_MFEM_ENABLED
#include "mfem/config/config.hpp"
#endif

#ifdef LIBMESH_HAVE_NETGEN
#include "netgen/netgen_version.hpp"
#endif

namespace Moose::internal
{

Capabilities::Capabilities() : CapabilityRegistry()
{
  // Register the moose capabilities, just once
  registerMooseCapabilities();
}

Capabilities &
Capabilities::get(const Capabilities::GetPassKey)
{
  // We need a naked new here (_not_ a smart pointer or object instance) due to what seems like a
  // bug in clang's static object destruction when using dynamic library loading.
  static Capabilities * capability_registry = nullptr;
  if (!capability_registry)
    capability_registry = new Capabilities();
  return *capability_registry;
}

std::string
Capabilities::dump() const
{
  nlohmann::json root;
  for (const auto & [name, capability] : _registry)
  {
    auto & entry = root[name];
    std::visit([&entry](const auto & v) { entry["value"] = v; }, capability.getValue());
    entry["doc"] = capability.getDoc();
    if (const auto enumeration_ptr = capability.queryEnumeration())
      entry["enumeration"] = *enumeration_ptr;
    if (!capability.hasBoolValue())
      entry["explicit"] = capability.getExplicit();
  }
  return root.dump(2);
}

void
Capabilities::augment(const nlohmann::json & input, const Capabilities::AugmentPassKey)
{
  for (const auto & [name_json, entry] : input.items())
  {
    const std::string name = name_json;

    const auto error = [&name](const std::string & message)
    { throw CapabilityException("Capabilities::augment: Capability '" + name + "' " + message); };

    std::string doc;
    if (const auto it = entry.find("doc"); it != entry.end())
      doc = *it;
    else
      error("missing 'doc' entry");

    Moose::Capability::Value value;
    if (const auto it = entry.find("value"); it != entry.end())
    {
      if (it->is_boolean())
        value = it->get<bool>();
      else if (it->is_number_integer())
        value = it->get<int>();
      else
        value = it->get<std::string>();
    }
    else
      error("missing 'value' entry");

    auto & capability = add(name, value, doc);

    if (const auto it = entry.find("explicit"); it != entry.end() && it->get<bool>())
      capability.setExplicit();

    if (const auto it = entry.find("enumeration"); it != entry.end())
      capability.setEnumeration(it->get<std::set<std::string>>());
  }
}

void
Capabilities::registerMooseCapabilities()
{
  // helper lambdas for explicitly adding typed capabilities
  const auto add_bool = [&](const std::string_view capability,
                            const bool value,
                            const std::string_view doc) -> Capability &
  { return add(capability, value, doc); };
  const auto add_int = [&](const std::string_view capability,
                           const int value,
                           const std::string_view doc) -> Capability &
  { return add(capability, value, doc); };
  const auto add_string = [&](const std::string_view capability,
                              const std::string_view value,
                              const std::string_view doc) -> Capability &
  { return add(capability, std::string(value), doc); };

  // helper lambdas for adding capabilities
  const auto have = [&](const std::string & capability, const std::string & doc) -> Capability &
  { return add_bool(capability, true, doc + " is available."); };
  const auto missing = [&](const std::string & capability,
                           const std::string & doc,
                           const std::string & help = "") -> Capability &
  { return add_bool(capability, false, doc + " is not available. " + help); };

  const auto have_version = [&](const std::string & capability,
                                const std::string & doc,
                                const std::string & version) -> Capability &
  { return add_string(capability, version, doc + " version " + version + " is available."); };
  const auto petsc_missing = [&](const std::string & capability,
                                 const std::string & doc) -> Capability &
  {
    return add_bool(
        capability, false, doc + " is not available. Check your PETSc configure options.");
  };
  const auto libmesh_missing = [&](const std::string & capability,
                                   const std::string & doc,
                                   const std::string & config_option) -> Capability &
  {
    return add_bool(capability,
                    false,
                    doc + " is not available. It is controlled by the `" + config_option +
                        "` libMesh configure option.");
  };

  {
    const auto doc = "LibTorch machine learning and parallel tensor algebra library";
#ifdef MOOSE_LIBTORCH_ENABLED
    add_string("libtorch", TORCH_VERSION, doc);
#else
    missing("libtorch",
            doc,
            "Check "
            "https://mooseframework.inl.gov/moose/getting_started/installation/"
            "install_libtorch.html for "
            "instructions on how to configure and build moose with libTorch.");
#endif
  }

  {
    const auto doc = "MFEM finite element library";
#ifdef MOOSE_MFEM_ENABLED
    add_string("mfem", MFEM_VERSION_STRING, doc);
#else
    missing("mfem",
            doc,
            "Install mfem using the scripts/update_and_rebuild_mfem.sh script after "
            "first running scripts/update_and_rebuild_conduit.sh. Finally, configure "
            "moose with ./configure --with-mfem");
#endif
  }

  {
    const auto doc = "New Engineering Material model Library, version 2";
#ifdef NEML2_ENABLED
    have("neml2", doc);
#else
    missing("neml2",
            doc,
            "Install neml2 using the scripts/update_and_rebuild_neml2.sh script, then "
            "configure moose with ./configure --with-neml2 --with-libtorch");
#endif
  }

  {
    const auto doc = "gperftools code performance analysis and profiling library";
#ifdef HAVE_GPERFTOOLS
    have("gperftools", doc);
#else
    missing("gperftools",
            doc,
            "Check https://mooseframework.inl.gov/application_development/profiling.html "
            "for instructions on profiling MOOSE based applications.");
#endif
  }

  {
    const auto doc = "libPNG portable network graphics format library";
#ifdef MOOSE_HAVE_LIBPNG
    have("libpng", doc);
#else
    missing("libpng",
            doc,
            "Install libpng through conda or your distribution and check that it gets "
            "detected through pkg-config, then reconfigure and rebuild MOOSE.");
#endif
  }

  {
    const auto doc = "NVIDIA GPU parallel computing platform";
#ifdef PETSC_HAVE_CUDA
    const std::string version = QUOTE(PETSC_PKG_CUDA_VERSION_MAJOR) "." QUOTE(
        PETSC_PKG_CUDA_VERSION_MINOR) "." QUOTE(PETSC_PKG_CUDA_VERSION_SUBMINOR);
    add_string("cuda", version, doc);
#else
    missing("cuda", doc, "Add the CUDA bin directory to your path and rebuild PETSc.");
#endif
  }

  {
    const auto doc = "Kokkos performance portability programming ecosystem";
#ifdef MOOSE_KOKKOS_ENABLED
    const std::string version = QUOTE(PETSC_PKG_KOKKOS_VERSION_MAJOR) "." QUOTE(
        PETSC_PKG_KOKKOS_VERSION_MINOR) "." QUOTE(PETSC_PKG_KOKKOS_VERSION_SUBMINOR);
    add_string("kokkos", version, doc);
#else
    missing("kokkos",
            doc,
            "Rebuild PETSc with Kokkos support and libMesh. Then, reconfigure MOOSE with "
            "--with-kokkos.");
#endif
  }

  {
    const auto doc = "Kokkos support for PETSc";
#ifdef PETSC_HAVE_KOKKOS
    const std::string version = QUOTE(PETSC_PKG_KOKKOS_VERSION_MAJOR) "." QUOTE(
        PETSC_PKG_KOKKOS_VERSION_MINOR) "." QUOTE(PETSC_PKG_KOKKOS_VERSION_SUBMINOR);
    add_string("petsc_kokkos", version, doc);
#else
    missing(
        "petsc_kokkos", doc, "Rebuild PETSc with Kokkos support, then rebuild libMesh and MOOSE.");
#endif
  }

  {
    const auto doc = "Intel OneAPI XPU accelerator support";
#ifdef MOOSE_HAVE_XPU
    if (torch::xpu::is_available())
      have("xpu", doc);
    else
      missing("xpu", doc, "No usable XPU devices have been found.");
#else
    missing("xpu", doc, "The torch version used to build this app has no XPU support.");
#endif
  }

  add_int(
      "ad_size",
      MOOSE_AD_MAX_DOFS_PER_ELEM,
      "MOOSE was configured and built with a dual number backing store size of " +
          Moose::stringify(MOOSE_AD_MAX_DOFS_PER_ELEM) +
          ". Complex simulations with many variables or contact problems may require larger "
          "values. Reconfigure MOOSE with the --with-derivative-size=<n> option in the root of the "
          "repository.")
      .setExplicit();

  {
    const std::string method = QUOTE(METHOD);
    add_string("method", method, "The executable was built with METHOD=\"" + method + "\"")
        .setExplicit()
        .setEnumeration({"dbg", "devel", "oprof", "opt"});
  }

  {
    const std::string version = QUOTE(LIBMESH_DETECTED_PETSC_VERSION_MAJOR) "." QUOTE(
        LIBMESH_DETECTED_PETSC_VERSION_MINOR) "." QUOTE(LIBMESH_DETECTED_PETSC_VERSION_SUBMINOR);
    add_string("petsc", version, "Using PETSc version " + version + ".");
  }

#ifdef LIBMESH_PETSC_USE_DEBUG
  add_bool("petsc_debug", true, "PETSc was built with debugging options.");
#else
  add_bool("petsc_debug", false, "PETSc was built without debugging options.");
#endif

  {
    const auto doc = "SuperLU direct solver";
#ifdef LIBMESH_PETSC_HAVE_SUPERLU_DIST
    const std::string version = QUOTE(PETSC_PKG_SUPERLU_DIST_VERSION_MAJOR) "." QUOTE(
        PETSC_PKG_SUPERLU_DIST_VERSION_MINOR) "." QUOTE(PETSC_PKG_SUPERLU_DIST_VERSION_SUBMINOR);
    add_string("superlu", version, doc);
#else
    petsc_missing("superlu", doc);
#endif
  }

  {
    const auto doc = "MUltifrontal Massively Parallel sparse direct Solver (MUMPS)";
#ifdef LIBMESH_PETSC_HAVE_MUMPS
    const std::string version = QUOTE(PETSC_PKG_MUMPS_VERSION_MAJOR) "." QUOTE(
        PETSC_PKG_MUMPS_VERSION_MINOR) "." QUOTE(PETSC_PKG_MUMPS_VERSION_SUBMINOR);
    add_string("mumps", version, doc);
#else
    petsc_missing("mumps", doc);
#endif
  }

  {
    const auto doc = "STRUMPACK - STRUctured Matrix PACKage solver library";
#ifdef LIBMESH_PETSC_HAVE_STRUMPACK
    const std::string version = QUOTE(PETSC_PKG_STRUMPACK_VERSION_MAJOR) "." QUOTE(
        PETSC_PKG_STRUMPACK_VERSION_MINOR) "." QUOTE(PETSC_PKG_STRUMPACK_VERSION_SUBMINOR);
    add_string("strumpack", version, doc);
#else
    petsc_missing("strumpack", doc);
#endif
  }

  {
    const auto doc = "Parmetis partitioning library";
#if defined(LIBMESH_PETSC_HAVE_PARMETIS) || defined(LIBMESH_HAVE_PARMETIS)
    const std::string version = QUOTE(PETSC_PKG_PARMETIS_VERSION_MAJOR) "." QUOTE(
        PETSC_PKG_PARMETIS_VERSION_MINOR) "." QUOTE(PETSC_PKG_PARMETIS_VERSION_SUBMINOR);
    add_string("parmetis", version, doc);
#else
    petsc_missing("parmetis", doc);
#endif
  }

  {
    const auto doc = "Chaco graph partitioning library";
#ifdef LIBMESH_PETSC_HAVE_CHACO
    have("chaco", doc);
#else
    petsc_missing("chaco", doc);
#endif
  }

  {
    const auto doc = "Party matrix or graph partitioning library";
#ifdef LIBMESH_PETSC_HAVE_PARTY
    have("party", doc);
#else
    petsc_missing("party", doc);
#endif
  }

  {
    const auto doc = "PT-Scotch graph partitioning library";
#ifdef LIBMESH_PETSC_HAVE_PTSCOTCH
    const std::string version = QUOTE(PETSC_PKG_PTSCOTCH_VERSION_MAJOR) "." QUOTE(
        PETSC_PKG_PTSCOTCH_VERSION_MINOR) "." QUOTE(PETSC_PKG_PTSCOTCH_VERSION_SUBMINOR);
    add_string("ptscotch", version, doc);
#else
    petsc_missing("ptscotch", doc);
#endif
  }

  {
    const auto doc = "Scalable Library for Eigenvalue Problem Computations (SLEPc)";
#ifdef LIBMESH_HAVE_SLEPC
    const auto version = QUOTE(LIBMESH_DETECTED_SLEPC_VERSION_MAJOR) "." QUOTE(
        LIBMESH_DETECTED_SLEPC_VERSION_MINOR) "." QUOTE(LIBMESH_DETECTED_SLEPC_VERSION_SUBMINOR);
    have_version("slepc", doc, version);
#else
    petsc_missing("slepc", doc);
#endif
  }

  {
    const auto doc = "Exodus mesh file format library";
#ifdef LIBMESH_HAVE_EXODUS_API
    const std::string version = QUOTE(LIBMESH_DETECTED_EXODUS_VERSION_MAJOR) "." QUOTE(
        LIBMESH_DETECTED_EXODUS_VERSION_MINOR);
    have_version("exodus", doc, version);
#else
    libmesh_missing("exodus", doc, "--enable-exodus");
#endif
  }

  {
    const auto doc = "Netgen meshing library";
#ifdef LIBMESH_HAVE_NETGEN
    const std::string version =
        QUOTE(NETGEN_VERSION_MAJOR) "." QUOTE(NETGEN_VERSION_MINOR) "." QUOTE(NETGEN_VERSION_PATCH);
    add_string("netgen", version, doc);
#else
    libmesh_missing("netgen", doc, "--enable-netgen");
#endif
  }

  {
    const auto doc = "Visualization Toolkit (VTK)";
#ifdef LIBMESH_HAVE_VTK
    const std::string version = QUOTE(LIBMESH_DETECTED_VTK_VERSION_MAJOR) "." QUOTE(
        LIBMESH_DETECTED_VTK_VERSION_MINOR) "." QUOTE(LIBMESH_DETECTED_VTK_VERSION_SUBMINOR);
    have_version("vtk", doc, version);
#else
    libmesh_missing("vtk", doc, "--disable-vtk and --enable-vtk-required");
#endif
  }

  {
    const auto doc = "libcurl - the multiprotocol file transfer library";
#ifdef LIBMESH_HAVE_CURL
    have("curl", doc);
#else
    libmesh_missing("curl", doc, "--enable-curl");
#endif
  }

  {
    const auto doc = "Tecplot post-processing tools API";
#ifdef LIBMESH_HAVE_TECPLOT_API
    have("tecplot", doc);
#else
    libmesh_missing("tecplot", doc, "--enable-tecplot");
#endif
  }

  {
    const auto doc = "Boost C++ library";
#ifdef LIBMESH_HAVE_EXTERNAL_BOOST
    have("boost", doc);
#else
    libmesh_missing("boost", doc, "--with-boost");
#endif
  }

  // libmesh stuff
  {
    const auto doc = "Adaptive mesh refinement";
#ifdef LIBMESH_ENABLE_AMR
    have("amr", doc);
#else
    libmesh_missing("amr", doc, "--disable-amr");
#endif
  }

  {
    const auto doc = "nanoflann library for Nearest Neighbor (NN) search with KD-trees";
#ifdef LIBMESH_HAVE_NANOFLANN
    have("nanoflann", doc);
#else
    libmesh_missing("nanoflann", doc, "--disable-nanoflann");
#endif
  }

  {
    const auto doc = "sfcurves library for space filling curves (required by geometric "
                     "partitioners such as SFCurves, Hilbert and Morton -  not LGPL compatible)";
#ifdef LIBMESH_HAVE_SFCURVES
    have("sfcurves", doc);
#else
    libmesh_missing("sfcurves", doc, "--disable-sfc");
#endif
  }

  {
#ifdef LIBMESH_HAVE_FPARSER
#ifdef LIBMESH_HAVE_FPARSER_JIT
    const auto value = "jit";
    const auto doc = "FParser enabled with just in time compilation support.";
#else
    const auto value = "byte_code";
    const auto doc = "FParser enabled.";
#endif
    add_string("fparser", value, doc);
#else
    add_bool("fparser",
             false,
             "FParser is disabled, libMesh was likely configured with --disable-fparser.");
#endif
  }

#ifdef LIBMESH_HAVE_DLOPEN
  add_bool("dlopen", true, "The dlopen() system call is available to dynamically load libraries.");
#else
  add_bool("dlopen",
           false,
           "The dlopen() system call is not available. Dynamic library loading is "
           "not supported on this system.");
#endif

  {
    const auto doc = "LibMesh support for threaded execution";
#ifdef LIBMESH_USING_THREADS
    have("threads", doc);
#else
    libmesh_missing("threads", doc, "--with-thread-model=tbb,pthread,openmp,auto,none");
#endif
  }

  {
    const auto doc = "OpenMP multi-platform shared-memory parallel programming API";
#ifdef LIBMESH_HAVE_OPENMP
    have("openmp", doc);
#else
    libmesh_missing("openmp", doc, "--with-thread-model=tbb,pthread,openmp,auto,none");
#endif
  }
  {
    const auto doc = "POSIX Threads API";
#ifdef LIBMESH_HAVE_PTHREAD
    have("pthread", doc);
#else
    libmesh_missing("pthread", doc, "--with-thread-model=tbb,pthread,openmp,auto,none");
#endif
  }
  {
    const auto doc = "oneAPI Threading Building Blocks (TBB) API";
#ifdef LIBMESH_HAVE_TBB_API
    have("tbb", doc);
#else
    libmesh_missing("tbb", doc, "--with-thread-model=tbb,pthread,openmp,auto,none");
#endif
  }

  {
    const auto doc = "libMesh unique ID support";
#ifdef LIBMESH_ENABLE_UNIQUE_ID
    have("unique_id", doc);
#else
    libmesh_missing("unique_id", doc, "--enable-unique-id");
#endif
  }

  {
#ifdef LIBMESH_ENABLE_PARMESH
    const auto value = "distributed";
#else
    const auto value = "replicated";
#endif
    add_string("mesh_mode", value, "libMesh default mesh mode")
        .setExplicit()
        .setEnumeration({"distributed", "replicated"});
  }

  add_int("dof_id_bytes",
          static_cast<int>(sizeof(dof_id_type)),
          "Degree of freedom (DOF) identifiers use " + Moose::stringify(sizeof(dof_id_type)) +
              " bytes for storage. This is controlled by the "
              "--with-dof-id-bytes=<1|2|4|8> libMesh configure option.")
      .setExplicit();

  // compiler
  {
    const auto doc = "Compiler used to build the MOOSE framework.";
#if defined(__INTEL_LLVM_COMPILER)
    const auto value = "intel";
#elif defined(__clang__)
    const auto value = "clang";
#elif defined(__GNUC__) || defined(__GNUG__)
    const auto value = "gcc";
#elif defined(_MSC_VER)
    const auto value = "msvc";
#else
    mooseDoOnce(mooseWarning("Failed to determine compiler; setting capability compiler=unknown"));
    const auto value = "unknown";
#endif
    add_string("compiler", value, doc)
        .setExplicit()
        .setEnumeration({"clang", "gcc", "intel", "msvc", "unknown"});
  }

  // OS related
  {
#ifdef __APPLE__
    const auto value = "darwin";
#elif __WIN32__
    const auto value = "win32";
#elif __linux__
    const auto value = "linux";
#elif __unix__
    const auto value = "unix";
#else
    mooseDoOnce(mooseWarning("Failed to determine platform; setting capability platform=unknown"));
    const auto value = "unknown";
#endif
    add_string("platform", value, "Operating system this executable is running on.")
        .setExplicit()
        .setEnumeration({"darwin", "linux", "unix", "unknown", "win32"});
  }

  // Installation type (in tree or installed)
  {
    // Try to find the path to the running executable
    std::optional<std::string> executable_path;
    {
      Moose::ScopedThrowOnError scoped_throw_on_error;
      try
      {
        executable_path = Moose::getExec();
      }
      catch (const MooseException &)
      {
      }
    }

    std::string value = "unknown";

    if (executable_path)
    {
      // Try to follow all symlinks to get the real path
      std::error_code ec;
      const auto resolved_path =
          std::filesystem::weakly_canonical(std::filesystem::path(*executable_path), ec);
      if (ec)
        mooseDoOnce(mooseWarning("Failed to resolve executable path '",
                                 *executable_path,
                                 "':\n",
                                 ec.message(),
                                 "\n\nSetting capability installation_type=unknown"));
      else
      {
        // If the binary is in a folder "bin", we'll consider it installed.
        // This isn't the best check, but it works with how we currently
        // install applications in app.mk
        value = resolved_path.parent_path().filename() == "bin" ? "relocated" : "in_tree";
      }
    }
    else
      mooseDoOnce(mooseWarning(
          "Failed to determine executable path; setting capability installation_type=unknown"));

    add_string("installation_type", value, "The installation type of the application.")
        .setExplicit()
        .setEnumeration({"in_tree", "relocated", "unknown"});
  }
}

} // namespace Moose
