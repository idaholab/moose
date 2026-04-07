//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "libmesh/libmesh_config.h"

// MOOSE_KOKKOS_NATIVE_FE enables the native (non-FEBase) shape function
// evaluation path for LAGRANGE and MONOMIAL elements.
//
// When libMesh is built with Kokkos support (LIBMESH_HAVE_KOKKOS), the FE
// math infrastructure lives in libMesh and this flag is set automatically.
// It can still be forced off by defining MOOSE_NO_KOKKOS_NATIVE_FE before
// including this header (useful for debugging the libMesh-fallback path).
#ifdef LIBMESH_HAVE_KOKKOS
#  ifndef MOOSE_NO_KOKKOS_NATIVE_FE
#    ifndef MOOSE_KOKKOS_NATIVE_FE
#      define MOOSE_KOKKOS_NATIVE_FE
#    endif
#  endif
#endif
