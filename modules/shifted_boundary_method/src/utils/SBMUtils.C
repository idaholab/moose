//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SBMUtils.h"
#include "Function.h"
#include "FunctionInterface.h"
#include "MooseParsedFunction.h"
#include "UnsignedDistanceToSurfaceMesh.h"

#include <limits>

namespace SBMUtils
{

bool
checkWatertightnessFromRawElems(const std::vector<const Elem *> & bd_elements)
{
  for (const auto * el : bd_elements)
    for (unsigned int s = 0; s < el->n_sides(); ++s)
      if (!el->neighbor_ptr(s))
        return false;

  return true;
}

std::vector<const Function *>
buildDistanceFunctions(const std::vector<FunctionName> & function_names,
                       const FunctionInterface & function_provider)
{
  std::vector<const Function *> funcs;
  funcs.reserve(function_names.size());

  for (const auto & name : function_names)
  {
    const Function * func = &function_provider.getFunctionByName(name);
    if (!dynamic_cast<const MooseParsedFunction *>(func) &&
        !dynamic_cast<const UnsignedDistanceToSurfaceMesh *>(func))
    {
      mooseError("SBM distance helpers only support ParsedFunction or "
                 "UnsignedDistanceToSurfaceMesh types. Offending function: ",
                 name);
    }
    funcs.emplace_back(func);
  }

  return funcs;
}

RealVectorValue
distanceVectorFromFunction(const Function * func, const libMesh::Point & pt, Real t)
{
  mooseAssert(dynamic_cast<const MooseParsedFunction *>(func) ||
                  dynamic_cast<const UnsignedDistanceToSurfaceMesh *>(func),
              "Function was not a valid distance strategy, the only "
              "supported types are ParsedFunction and "
              "UnsignedDistanceToSurfaceMesh.");

  const Real phi = func->value(t, pt);
  const RealVectorValue grad_phi = func->gradient(t, pt);
  const Real grad_norm = grad_phi.norm();

  if (grad_norm <= libMesh::TOLERANCE)
    return RealVectorValue(0.0, 0.0, 0.0);

  return -(phi / grad_norm) * grad_phi;
}

RealVectorValue
trueNormalFromFunction(const Function * func, const libMesh::Point & pt, Real t)
{
  if (const auto * parsed = dynamic_cast<const MooseParsedFunction *>(func))
  {
    const auto proj_pt = pt + distanceVectorFromFunction(func, pt, t);
    const RealVectorValue grad_phi = parsed->gradient(t, proj_pt);
    const Real grad_norm = grad_phi.norm();
    if (grad_norm <= libMesh::TOLERANCE)
      return RealVectorValue(0.0, 0.0, 0.0);
    return grad_phi / grad_norm;
  }
  else
  {
    const auto * mesh_func = dynamic_cast<const UnsignedDistanceToSurfaceMesh *>(func);
    mooseAssert(mesh_func, "Function was not a valid distance strategy");
    return mesh_func->surfaceNormal(pt);
  }
}

RealVectorValue
closestDistanceVector(const std::vector<const Function *> & funcs,
                      const libMesh::Point & pt,
                      Real t)
{
  Real min_dist = std::numeric_limits<Real>::max();
  RealVectorValue closest_dist_vec;

  for (const auto & func : funcs)
  {
    const auto dist_vec = distanceVectorFromFunction(func, pt, t);
    const auto dist = dist_vec.norm();
    if (dist < min_dist)
    {
      min_dist = dist;
      closest_dist_vec = dist_vec;
    }
  }

  return closest_dist_vec;
}

RealVectorValue
closestTrueNormalVector(const std::vector<const Function *> & funcs,
                        const libMesh::Point & pt,
                        Real t)
{
  Real min_dist = std::numeric_limits<Real>::max();
  RealVectorValue closest_normal_vec;

  for (const auto & func : funcs)
  {
    const auto dist_vec = distanceVectorFromFunction(func, pt, t);
    const auto dist = dist_vec.norm();
    if (dist < min_dist)
    {
      min_dist = dist;
      closest_normal_vec = trueNormalFromFunction(func, pt, t);
    }
  }

  return closest_normal_vec;
}

} // namespace SBMUtils
