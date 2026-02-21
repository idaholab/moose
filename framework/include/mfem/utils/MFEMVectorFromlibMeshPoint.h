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

#include "libmesh/utility.h"
#include "mfem.hpp"

namespace Moose::MFEM
{
/**
 * Utilities for converting between vector(s) of libMesh Points and MFEM Vector(s).
 */
size_t
MFEMIndex(const size_t i_dim,
          const size_t i_point,
          const size_t num_dims,
          const size_t num_points,
          const mfem::Ordering::Type ordering)
{
  if (ordering == mfem::Ordering::byNODES)
  {
    return i_dim * num_points + i_point;
  }
  else // ordering == mfem::Ordering::byVDIM
  {
    return i_point * num_dims + i_dim;
  }
}

mfem::Vector
pointsToMFEMVector(const std::vector<Point> & points,
                   const unsigned int num_dims,
                   const mfem::Ordering::Type ordering)
{
  const unsigned int num_points = points.size();
  mfem::Vector mfem_points(num_points * num_dims);
  for (unsigned int i_point = 0; i_point < num_points; i_point++)
  {
    for (unsigned int i_dim = 0; i_dim < num_dims; i_dim++)
    {
      const size_t idx = MFEMIndex(i_dim, i_point, num_dims, num_points, ordering);

      mfem_points(idx) = points[i_point](i_dim);
    }
  }

  return mfem_points;
}

}

#endif
