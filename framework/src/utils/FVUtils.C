//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVUtils.h"
#include "MooseVariableFV.h"

namespace Moose
{
namespace FV
{
template <typename T, typename T2>
typename ADType<T>::type
gradUDotNormal(const typename ADType<T>::type &
#ifndef MOOSE_GLOBAL_AD_INDEXING
                   elem_value
#endif
               ,
               const T2 &
#ifndef MOOSE_GLOBAL_AD_INDEXING
                   neighbor_value
#endif
               ,
               const FaceInfo & face_info,
               const MooseVariableFV<T> &
#ifdef MOOSE_GLOBAL_AD_INDEXING
                   fv_var
#endif
)
{
#ifdef MOOSE_GLOBAL_AD_INDEXING
  return fv_var.adGradSln(face_info) * face_info.normal();
#else

  // Orthogonal contribution
  auto orthogonal = (neighbor_value - elem_value) * (1 / face_info.dCFMag());

  return orthogonal; // TO-DO for local indexing: add non-orthogonal contribution
#endif
}

template <>
ADRealEigenVector
gradUDotNormal(const ADRealEigenVector & elem_value,
               const ADRealEigenVector & neighbor_value,
               const FaceInfo & face_info,
               const MooseVariableFV<RealEigenVector> & /*fv_var*/)
{
  // TODO: update this function to use fv gradient reconstruction once we add support for
  // it for array variables.

  // Orthogonal contribution
  auto orthogonal = (neighbor_value - elem_value) * (1 / face_info.dCFMag());

  return orthogonal; // TO-DO for local indexing: add non-orthogonal contribution
}

template ADReal
gradUDotNormal(const ADReal &, const ADReal &, const FaceInfo &, const MooseVariableFV<Real> &);
template ADReal
gradUDotNormal(const ADReal &, const Real &, const FaceInfo &, const MooseVariableFV<Real> &);
}
}
