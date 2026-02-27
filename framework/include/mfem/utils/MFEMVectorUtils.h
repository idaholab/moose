//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#pragma once
#include <vector>

#include "libmesh/point.h"
#include "libmesh/ignore_warnings.h"
#include "mfem.hpp"
#include "libmesh/restore_warnings.h"

/**
 * Utilities for converting between vector(s) of libMesh Points and MFEM Vector(s).
 */
namespace Moose::MFEM
{
/**
 * Convert an index of a vector of libMesh::Points to an MFEM vector index, given an MFEM ordering
 */
size_t MFEMIndex(const size_t i_dim,
                 const size_t i_point,
                 const size_t num_dims,
                 const size_t num_points,
                 const mfem::Ordering::Type ordering);

/**
 * Convert an MFEM position vector to a libMesh::Point.
 */
libMesh::Point libMeshPointFromMFEMVector(const mfem::Vector & vec);

/**
 * Convert a vector of libMesh::Point objects to an mfem::Vector containing all points, given an
 * ordering.
 */
mfem::Vector libMeshPointsToMFEMVector(const std::vector<libMesh::Point> & points,
                                       const unsigned int num_dims,
                                       const mfem::Ordering::Type ordering);

}

#endif
