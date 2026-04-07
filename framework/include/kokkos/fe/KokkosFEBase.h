//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// Forwarding shim — the canonical header is now libmesh/kokkos/fe_base.h.
// This file exists for backward compatibility with MOOSE code that includes
// the old path.  It will be removed once all callers are updated.

#pragma once

#include "libmesh/kokkos/fe_base.h"

namespace Moose::Kokkos
{
using namespace libMesh::Kokkos;
} // namespace Moose::Kokkos
