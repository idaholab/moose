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

namespace Moose::MFEM
{
inline size_t
MFEMIndex(const size_t i_dim,
          const size_t i_point,
          const size_t num_dims,
          const size_t num_points,
          const mfem::Ordering::Type ordering)
{
  if (ordering == mfem::Ordering::byNODES)
    return mfem::Ordering::Map<mfem::Ordering::byNODES>(num_points, num_dims, i_point, i_dim);
  else
    return mfem::Ordering::Map<mfem::Ordering::byVDIM>(num_points, num_dims, i_point, i_dim);
}

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
  for (const auto i_point : make_range(num_points))
    for (const auto i_dim : make_range(num_dims))
      mfem_points(MFEMIndex(i_dim, i_point, num_dims, num_points, ordering)) =
          points[i_point](i_dim);

  return mfem_points;
}
}

#endif
