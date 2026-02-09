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
  // helper lambdas
  const auto add_bool_capability =
      [&](const std::string_view capability, const bool value, const std::string_view doc)
  { return add(capability, value, doc); };
  const auto add_int_capability =
      [&](const std::string_view capability, const int value, const std::string_view doc)
  { return add(capability, value, doc); };
  const auto add_string_capability = [&](const std::string_view capability,
                                         const std::string_view value,
                                         const std::string_view doc)
  { return add(capability, std::string(value), doc); };
  auto have_capability = [&](const std::string & capability, const std::string & doc)
  { return add_bool_capability(capability, true, doc + " is available."); };
  auto missing_capability =
      [&](const std::string & capability, const std::string & doc, const std::string & help = "")
  { return add_bool_capability(capability, false, doc + " is not available. " + help); };

  auto have_capability_version =
      [&](const std::string & capability, const std::string & doc, const std::string & version)
  {
    return add_string_capability(
        capability, version, doc + " version " + version + " is available.");
  };
  auto petsc_missing_capability = [&](const std::string & capability, const std::string & doc)
  {
    return add_bool_capability(
        capability, false, doc + " is not available. Check your PETSc configure options.");
  };
  auto libmesh_missing_capability = [&](const std::string & capability,
                                        const std::string & doc,
                                        const std::string & config_option)
  {
    return add_bool_capability(capability,
                               false,
                               doc + " is not available. It is controlled by the `" +
                                   config_option + "` libMesh configure option.");
  };

  {
    const auto doc = "LibTorch machine learning and parallel tensor algebra library";
#ifdef MOOSE_LIBTORCH_ENABLED
    add_string_capability("libtorch", TORCH_VERSION, doc);
#else
    missing_capability("libtorch",
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
    add_string_capability("mfem", MFEM_VERSION_STRING, doc);
#else
    missing_capability("mfem",
                       doc,
                       "Install mfem using the scripts/update_and_rebuild_mfem.sh script after "
                       "first running scripts/update_and_rebuild_conduit.sh. Finally, configure "
                       "moose with ./configure --with-mfem");
#endif
  }

  {
    const auto doc = "New Engineering Material model Library, version 2";
#ifdef NEML2_ENABLED
    have_capability("neml2", doc);
#else
    missing_capability("neml2",
                       doc,
                       "Install neml2 using the scripts/update_and_rebuild_neml2.sh script, then "
                       "configure moose with ./configure --with-neml2 --with-libtorch");
#endif
  }

  {
    const auto doc = "gperftools code performance analysis and profiling library";
#ifdef HAVE_GPERFTOOLS
    have_capability("gperftools", doc);
#else
    missing_capability(
        "gperftools",
        doc,
        "Check https://mooseframework.inl.gov/application_development/profiling.html "
        "for instructions on profiling MOOSE based applications.");
#endif
  }

  {
    const auto doc = "libPNG portable network graphics format library";
#ifdef MOOSE_HAVE_LIBPNG
    have_capability("libpng", doc);
#else
    missing_capability("libpng",
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
    add_string_capability("cuda", version, doc);
#else
    missing_capability("cuda", doc, "Add the CUDA bin directory to your path and rebuild PETSc.");
#endif
  }

  {
    const auto doc = "Kokkos performance portability programming ecosystem";
#ifdef MOOSE_KOKKOS_ENABLED
    const std::string version = QUOTE(PETSC_PKG_KOKKOS_VERSION_MAJOR) "." QUOTE(
        PETSC_PKG_KOKKOS_VERSION_MINOR) "." QUOTE(PETSC_PKG_KOKKOS_VERSION_SUBMINOR);
    add_string_capability("kokkos", version, doc);
#else
    missing_capability(
        "kokkos",
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
    add_string_capability("petsc_kokkos", version, doc);
#else
    missing_capability(
        "petsc_kokkos", doc, "Rebuild PETSc with Kokkos support, then rebuild libMesh and MOOSE.");
#endif
  }

  {
    const auto doc = "Intel OneAPI XPU accelerator support";
#ifdef MOOSE_HAVE_XPU
    if (torch::xpu::is_available())
      have_capability("xpu", doc);
    else
      missing_capability("xpu", doc, "No usable XPU devices have been found.");
#else
    missing_capability("xpu", doc, "The torch version used to build this app has no XPU support.");
#endif
  }

  add_int_capability(
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
    add_string_capability(
        "method", method, "The executable was built with METHOD=\"" + method + "\"")
        .setExplicit()
        .setEnumeration({"dbg", "devel", "oprof", "opt"});
  }

  {
    const std::string version = QUOTE(LIBMESH_DETECTED_PETSC_VERSION_MAJOR) "." QUOTE(
        LIBMESH_DETECTED_PETSC_VERSION_MINOR) "." QUOTE(LIBMESH_DETECTED_PETSC_VERSION_SUBMINOR);
    add_string_capability("petsc", version, "Using PETSc version " + version + ".");
  }

#ifdef LIBMESH_PETSC_USE_DEBUG
  add_bool_capability("petsc_debug", true, "PETSc was built with debugging options.");
#else
  add_bool_capability("petsc_debug", false, "PETSc was built without debugging options.");
#endif

  {
    const auto doc = "SuperLU direct solver";
#ifdef LIBMESH_PETSC_HAVE_SUPERLU_DIST
    const std::string version = QUOTE(PETSC_PKG_SUPERLU_DIST_VERSION_MAJOR) "." QUOTE(
        PETSC_PKG_SUPERLU_DIST_VERSION_MINOR) "." QUOTE(PETSC_PKG_SUPERLU_DIST_VERSION_SUBMINOR);
    add_string_capability("superlu", version, doc);
#else
    petsc_missing_capability("superlu", doc);
#endif
  }

  {
    const auto doc = "MUltifrontal Massively Parallel sparse direct Solver (MUMPS)";
#ifdef LIBMESH_PETSC_HAVE_MUMPS
    const std::string version = QUOTE(PETSC_PKG_MUMPS_VERSION_MAJOR) "." QUOTE(
        PETSC_PKG_MUMPS_VERSION_MINOR) "." QUOTE(PETSC_PKG_MUMPS_VERSION_SUBMINOR);
    add_string_capability("mumps", version, doc);
#else
    petsc_missing_capability("mumps", doc);
#endif
  }

  {
    const auto doc = "STRUMPACK - STRUctured Matrix PACKage solver library";
#ifdef LIBMESH_PETSC_HAVE_STRUMPACK
    const std::string version = QUOTE(PETSC_PKG_STRUMPACK_VERSION_MAJOR) "." QUOTE(
        PETSC_PKG_STRUMPACK_VERSION_MINOR) "." QUOTE(PETSC_PKG_STRUMPACK_VERSION_SUBMINOR);
    add_string_capability("strumpack", version, doc);
#else
    petsc_missing_capability("strumpack", doc);
#endif
  }

  {
    const auto doc = "Parmetis partitioning library";
#if defined(LIBMESH_PETSC_HAVE_PARMETIS) || defined(LIBMESH_HAVE_PARMETIS)
    const std::string version = QUOTE(PETSC_PKG_PARMETIS_VERSION_MAJOR) "." QUOTE(
        PETSC_PKG_PARMETIS_VERSION_MINOR) "." QUOTE(PETSC_PKG_PARMETIS_VERSION_SUBMINOR);
    add_string_capability("parmetis", version, doc);
#else
    petsc_missing_capability("parmetis", doc);
#endif
  }

  {
    const auto doc = "Chaco graph partitioning library";
#ifdef LIBMESH_PETSC_HAVE_CHACO
    have_capability("chaco", doc);
#else
    petsc_missing_capability("chaco", doc);
#endif
  }

  {
    const auto doc = "Party matrix or graph partitioning library";
#ifdef LIBMESH_PETSC_HAVE_PARTY
    have_capability("party", doc);
#else
    petsc_missing_capability("party", doc);
#endif
  }

  {
    const auto doc = "PT-Scotch graph partitioning library";
#ifdef LIBMESH_PETSC_HAVE_PTSCOTCH
    const std::string version = QUOTE(PETSC_PKG_PTSCOTCH_VERSION_MAJOR) "." QUOTE(
        PETSC_PKG_PTSCOTCH_VERSION_MINOR) "." QUOTE(PETSC_PKG_PTSCOTCH_VERSION_SUBMINOR);
    add_string_capability("ptscotch", version, doc);
#else
    petsc_missing_capability("ptscotch", doc);
#endif
  }

  {
    const auto doc = "Scalable Library for Eigenvalue Problem Computations (SLEPc)";
#ifdef LIBMESH_HAVE_SLEPC
    const auto version = QUOTE(LIBMESH_DETECTED_SLEPC_VERSION_MAJOR) "." QUOTE(
        LIBMESH_DETECTED_SLEPC_VERSION_MINOR) "." QUOTE(LIBMESH_DETECTED_SLEPC_VERSION_SUBMINOR);
    have_capability_version("slepc", doc, version);
#else
    petsc_missing_capability("slepc", doc);
#endif
  }

  {
    const auto doc = "Exodus mesh file format library";
#ifdef LIBMESH_HAVE_EXODUS_API
    const std::string version = QUOTE(LIBMESH_DETECTED_EXODUS_VERSION_MAJOR) "." QUOTE(
        LIBMESH_DETECTED_EXODUS_VERSION_MINOR);
    have_capability_version("exodus", doc, version);
#else
    libmesh_missing_capability("exodus", doc, "--enable-exodus");
#endif
  }

  {
    const auto doc = "Netgen meshing library";
#ifdef LIBMESH_HAVE_NETGEN
    const std::string version =
        QUOTE(NETGEN_VERSION_MAJOR) "." QUOTE(NETGEN_VERSION_MINOR) "." QUOTE(NETGEN_VERSION_PATCH);
    add_string_capability("netgen", version, doc);
#else
    libmesh_missing_capability("netgen", doc, "--enable-netgen");
#endif
  }

  {
    const auto doc = "Visualization Toolkit (VTK)";
#ifdef LIBMESH_HAVE_VTK
    const std::string version = QUOTE(LIBMESH_DETECTED_VTK_VERSION_MAJOR) "." QUOTE(
        LIBMESH_DETECTED_VTK_VERSION_MINOR) "." QUOTE(LIBMESH_DETECTED_VTK_VERSION_SUBMINOR);
    have_capability_version("vtk", doc, version);
#else
    libmesh_missing_capability("vtk", doc, "--disable-vtk and --enable-vtk-required");
#endif
  }

  {
    const auto doc = "libcurl - the multiprotocol file transfer library";
#ifdef LIBMESH_HAVE_CURL
    have_capability("curl", doc);
#else
    libmesh_missing_capability("curl", doc, "--enable-curl");
#endif
  }

  {
    const auto doc = "Tecplot post-processing tools API";
#ifdef LIBMESH_HAVE_TECPLOT_API
    have_capability("tecplot", doc);
#else
    libmesh_missing_capability("tecplot", doc, "--enable-tecplot");
#endif
  }

  {
    const auto doc = "Boost C++ library";
#ifdef LIBMESH_HAVE_EXTERNAL_BOOST
    have_capability("boost", doc);
#else
    libmesh_missing_capability("boost", doc, "--with-boost");
#endif
  }

  // libmesh stuff
  {
    const auto doc = "Adaptive mesh refinement";
#ifdef LIBMESH_ENABLE_AMR
    have_capability("amr", doc);
#else
    libmesh_missing_capability("amr", doc, "--disable-amr");
#endif
  }

  {
    const auto doc = "nanoflann library for Nearest Neighbor (NN) search with KD-trees";
#ifdef LIBMESH_HAVE_NANOFLANN
    have_capability("nanoflann", doc);
#else
    libmesh_missing_capability("nanoflann", doc, "--disable-nanoflann");
#endif
  }

  {
    const auto doc = "sfcurves library for space filling curves (required by geometric "
                     "partitioners such as SFCurves, Hilbert and Morton -  not LGPL compatible)";
#ifdef LIBMESH_HAVE_SFCURVES
    have_capability("sfcurves", doc);
#else
    libmesh_missing_capability("sfcurves", doc, "--disable-sfc");
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
    add_string_capability("fparser", value, doc);
#else
    add_bool_capability(
        "fparser",
        false,
        "FParser is disabled, libMesh was likely configured with --disable-fparser.");
#endif
  }

#ifdef LIBMESH_HAVE_DLOPEN
  add_bool_capability(
      "dlopen", true, "The dlopen() system call is available to dynamically load libraries.");
#else
  add_bool_capability("dlopen",
                      false,
                      "The dlopen() system call is not available. Dynamic library loading is "
                      "not supported on this system.");
#endif

  {
    const auto doc = "LibMesh support for threaded execution";
#ifdef LIBMESH_USING_THREADS
    have_capability("threads", doc);
#else
    libmesh_missing_capability("threads", doc, "--with-thread-model=tbb,pthread,openmp,auto,none");
#endif
  }

  {
    const auto doc = "OpenMP multi-platform shared-memory parallel programming API";
#ifdef LIBMESH_HAVE_OPENMP
    have_capability("openmp", doc);
#else
    libmesh_missing_capability("openmp", doc, "--with-thread-model=tbb,pthread,openmp,auto,none");
#endif
  }
  {
    const auto doc = "POSIX Threads API";
#ifdef LIBMESH_HAVE_PTHREAD
    have_capability("pthread", doc);
#else
    libmesh_missing_capability("pthread", doc, "--with-thread-model=tbb,pthread,openmp,auto,none");
#endif
  }
  {
    const auto doc = "oneAPI Threading Building Blocks (TBB) API";
#ifdef LIBMESH_HAVE_TBB_API
    have_capability("tbb", doc);
#else
    libmesh_missing_capability("tbb", doc, "--with-thread-model=tbb,pthread,openmp,auto,none");
#endif
  }

  {
    const auto doc = "libMesh unique ID support";
#ifdef LIBMESH_ENABLE_UNIQUE_ID
    have_capability("unique_id", doc);
#else
    libmesh_missing_capability("unique_id", doc, "--enable-unique-id");
#endif
  }

  {
#ifdef LIBMESH_ENABLE_PARMESH
    const auto value = "distributed";
#else
    const auto value = "replicated";
#endif
    add_string_capability("mesh_mode", value, "libMesh default mesh mode")
        .setExplicit()
        .setEnumeration({"distributed", "replicated"});
  }

  add_int_capability("dof_id_bytes",
                     static_cast<int>(sizeof(dof_id_type)),
                     "Degree of freedom (DOF) identifiers use " +
                         Moose::stringify(sizeof(dof_id_type)) +
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
    add_string_capability("compiler", value, doc)
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
    add_string_capability("platform", value, "Operating system this executable is running on.")
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

    add_string_capability("installation_type", value, "The installation type of the application.")
        .setExplicit()
        .setEnumeration({"in_tree", "relocated", "unknown"});
  }
}

} // namespace Moose
