//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#ifndef MOOSE_KOKKOS_SCOPE
#error Kokkos header was included in a source file which is not in the Kokkos compilation scope
#endif

// NVCC invokes compile error due to conflicting built-in and PETSc complex operators
// so this preprocessor should be defined
#define PETSC_SKIP_CXX_COMPLEX_FIX 1

// Kokkos headers
#include "Kokkos_Core.hpp"
#include "Kokkos_StdAlgorithms.hpp"

// libMesh uses an old Boost library and causes a build error due to an outdated CUDA
// preprocessor which should be manually undefined after including CUDA runtime header
#undef __CUDACC_VER__

// MOOSE includes
#include "MooseError.h"
#include "MooseUtils.h"
#include "MooseConfig.h"

// Architecture-dependent execution spaces
#ifdef MOOSE_ENABLE_KOKKOS_GPU
#ifdef KOKKOS_ENABLE_CUDA
#define MemSpace ::Kokkos::CudaSpace
#define ExecSpace ::Kokkos::Cuda
#endif
#ifdef KOKKOS_ENABLE_HIP
#define MemSpace ::Kokkos::HIPSpace
#define ExecSpace ::Kokkos::HIP
#endif
#ifdef KOKKOS_ENABLE_SYCL
#define MemSpace ::Kokkos::SYCLDeviceUSMSpace
#define ExecSpace ::Kokkos::SYCL
#endif
#else
#define MemSpace ::Kokkos::HostSpace
#define ExecSpace ::Kokkos::OpenMP
#undef KOKKOS_FUNCTION
#define KOKKOS_FUNCTION
#undef KOKKOS_INLINE_FUNCTION
#define KOKKOS_INLINE_FUNCTION inline
#undef KOKKOS_IF_ON_HOST
#define KOKKOS_IF_ON_HOST(code)                                                                    \
  if (!omp_get_level())                                                                            \
  {                                                                                                \
    code                                                                                           \
  }
#undef KOKKOS_IF_ON_DEVICE
#define KOKKOS_IF_ON_DEVICE(code)                                                                  \
  if (omp_get_level())                                                                             \
  {                                                                                                \
    code                                                                                           \
  }
#endif

namespace Moose
{
namespace Kokkos
{
// Predefined maximum values
constexpr unsigned int MAX_TAG = 30;
constexpr unsigned int MAX_DOF = 30;
} // namespace Kokkos
} // namespace Moose
