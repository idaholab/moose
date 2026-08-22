//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AdvectionHDGAssemblyHelper.h"

/**
 * Implements all the methods for assembling a hybridized interior penalty discontinuous Galerkin
 * (IPDG-H), which is a type of HDG method, discretization of the advection equation. These routines
 * may be called by both HDG kernels and integrated boundary conditions.
 */
class AdvectionIPHDGAssemblyHelper : public AdvectionHDGAssemblyHelper
{
public:
  static InputParameters validParams();

  AdvectionIPHDGAssemblyHelper(const MooseObject * const moose_obj,
                               MooseVariableDependencyInterface * const mvdi,
                               const TransientInterface * const ti,
                               SystemBase & sys,
                               const Assembly & assembly,
                               const THREAD_ID tid,
                               const std::set<SubdomainID> & block_ids,
                               const std::set<BoundaryID> & boundary_ids);

protected:
  virtual ADRealVectorValue faceVelocity(unsigned int qp) const override;

  /// Face evaluation of the H(div)-conforming cell velocity.
  const ADMaterialProperty<RealVectorValue> & _face_velocity;
};
