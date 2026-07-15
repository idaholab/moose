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
#include "ADFunctorInterface.h"

/**
 * Supplies a hybrid-velocity face evaluation to the shared hybridized DG advection assembly.
 */
class AdvectionLHDGAssemblyHelper : public AdvectionHDGAssemblyHelper, public ADFunctorInterface
{
public:
  static InputParameters validParams();

  AdvectionLHDGAssemblyHelper(const MooseObject * moose_obj,
                              MooseVariableDependencyInterface * mvdi,
                              const TransientInterface * ti,
                              SystemBase & sys,
                              const Assembly & assembly,
                              THREAD_ID tid,
                              const std::set<SubdomainID> & block_ids,
                              const std::set<BoundaryID> & boundary_ids);

  /// Constrains the unused L-HDG Dirichlet facet variable to zero.
  void lmDirichletZero();

protected:
  virtual ADRealVectorValue faceVelocity(unsigned int qp) const override;

  /// Hybrid velocity used in numerical face fluxes.
  const Moose::Functor<ADRealVectorValue> & _hybrid_velocity;
};
