//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MathFVUtils.h"
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
               ,
               bool
#ifdef MOOSE_GLOBAL_AD_INDEXING
                   correct_skewness
#endif
)

{
#ifdef MOOSE_GLOBAL_AD_INDEXING

  return fv_var.adGradSln(face_info, correct_skewness) * face_info.normal();
#else

  // Orthogonal contribution
  auto orthogonal = (neighbor_value - elem_value) / face_info.dCFMag();

  return orthogonal; // TO-DO for local indexing: add non-orthogonal contribution
#endif
}

bool
onBoundary(const std::set<SubdomainID> & subs, const FaceInfo & fi)
{
  if (!fi.neighborPtr())
    // We're on the exterior boundary
    return true;

  if (subs.empty())
    // The face is internal and our functor lives on all subdomains
    return false;

  const auto sub_count =
      subs.count(fi.elem().subdomain_id()) + subs.count(fi.neighbor().subdomain_id());

  switch (sub_count)
  {
    case 0:
      mooseError("We should not be calling isExtrapolatedBoundaryFace on a functor that doesn't "
                 "live on either of the face information's neighboring elements");

    case 1:
      // We only live on one of the subs
      return true;

    case 2:
      // We live on both of the subs
      return false;

    default:
      mooseError("There should be no other sub_count options");
  }
}

template ADReal gradUDotNormal(
    const ADReal &, const ADReal &, const FaceInfo &, const MooseVariableFV<Real> &, bool);
template ADReal
gradUDotNormal(const ADReal &, const Real &, const FaceInfo &, const MooseVariableFV<Real> &, bool);
}
}
