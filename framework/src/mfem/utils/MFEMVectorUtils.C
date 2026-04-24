//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMVectorUtils.h"
#include "libmesh/int_range.h"

namespace Moose::MFEM
{
libMesh::Point
libMeshPointFromMFEMVector(const mfem::Vector & vec)
{
  return libMesh::Point(vec(0), vec.Size() > 1 ? vec(1) : 0., vec.Size() > 2 ? vec(2) : 0.);
}

mfem::Vector
libMeshPointsToMFEMVector(const std::vector<libMesh::Point> & points,
                          const unsigned int num_dims,
                          const mfem::Ordering::Type ordering)
{
  const unsigned int num_points = points.size();
  mfem::Vector mfem_points(num_points * num_dims);
  for (const auto i_point : libMesh::make_range(num_points))
    for (const auto i_dim : libMesh::make_range(num_dims))
      mfem_points(MFEMIndex(i_dim, i_point, num_dims, num_points, ordering)) =
          points[i_point](i_dim);

  return mfem_points;
}
}

#endif
