//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseTypes.h"
#include "MooseError.h"
#include "FVUtils.h"

namespace NS
{
template <typename NSFVClass>
ADReal
coeffCalculator(const Elem * const elem, const NSFVClass & nsfv)
{
  ADReal coeff = 0;

  const auto * const u_var = nsfv.uVar();
  const auto * const v_var = nsfv.vVar();
  const auto * const w_var = nsfv.wVar();

  ADRealVectorValue elem_velocity(u_var->getElemValue(elem));

  if (v_var)
    elem_velocity(1) = v_var->getElemValue(elem);
  if (w_var)
    elem_velocity(2) = w_var->getElemValue(elem);

  auto action_functor = [&coeff, &elem_velocity, &u_var, &v_var, &w_var, &nsfv](
                            const Elem & /*functor_elem*/,
                            const Elem * const neighbor,
                            const FaceInfo * const fi,
                            const Point & surface_vector,
                            Real coord,
                            const bool /*elem_has_info*/) {
    mooseAssert(fi, "We need a non-null FaceInfo");
    ADRealVectorValue neighbor_velocity(u_var->getNeighborValue(neighbor, *fi, elem_velocity(0)));
    if (v_var)
      neighbor_velocity(1) = v_var->getNeighborValue(neighbor, *fi, elem_velocity(1));
    if (w_var)
      neighbor_velocity(2) = w_var->getNeighborValue(neighbor, *fi, elem_velocity(2));

    ADRealVectorValue interp_v;
    Moose::FV::interpolate(
        Moose::FV::InterpMethod::Average, interp_v, elem_velocity, neighbor_velocity, *fi);

    ADReal mass_flow = nsfv.rho() * interp_v * surface_vector;

    coeff += -mass_flow;

    // Now add the viscous flux
    coeff +=
        nsfv.mu() * fi->faceArea() * coord / (fi->elemCentroid() - fi->neighborCentroid()).norm();
  };

  const auto & subproblem = nsfv.subProblem();

  Moose::FV::loopOverElemFaceInfo(*elem, subproblem.mesh(), subproblem, action_functor);

  return coeff;
}
}
