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
ADReal
gradUDotNormal(const T &
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
               const MooseVariableFV<Real> &
#ifdef MOOSE_GLOBAL_AD_INDEXING
                   fv_var
#endif
)
{
#ifdef MOOSE_GLOBAL_AD_INDEXING
  return fv_var.adGradSln(face_info) * face_info.normal();
#else

  // Orthogonal contribution
  auto orthogonal = (neighbor_value - elem_value) / face_info.dCFMag();

  return orthogonal; // TO-DO for local indexing: add non-orthogonal contribution
#endif
}

template ADReal
gradUDotNormal(const ADReal &, const ADReal &, const FaceInfo &, const MooseVariableFV<Real> &);
template ADReal
gradUDotNormal(const ADReal &, const Real &, const FaceInfo &, const MooseVariableFV<Real> &);
}
}
