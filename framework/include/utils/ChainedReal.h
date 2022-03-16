//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "libmesh/libmesh_common.h"
#include "libmesh/compare_types.h"

#include "metaphysicl/metaphysicl_version.h"

namespace MetaPhysicL
{
#if METAPHYSICL_MAJOR_VERSION < 1
template <typename, typename>
class DualNumber;
#else
#include "metaphysicl/dualnumber_forward.h"
#endif
}

using libMesh::Real;
using MetaPhysicL::DualNumber;

typedef DualNumber<Real, Real> ChainedReal;
