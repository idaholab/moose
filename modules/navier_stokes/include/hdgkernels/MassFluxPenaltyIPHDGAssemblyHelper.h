//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "IPHDGAssemblyHelper.h"
#include "ADFunctorInterface.h"

/*
 * Imposes a singular perturbation on the component momentum equations penalizing discontinuities in
 * mass flux. Similar to \p MassFluxPenalty except it does not couple interior degrees of freedom on
 * neighboring elements, which makes this class useful in tandem with hybridized discretizations
 * because it supports static condensation
 */
class MassFluxPenaltyIPHDGAssemblyHelper : public IPHDGAssemblyHelper, public ADFunctorInterface
{
public:
  static InputParameters validParams();

  MassFluxPenaltyIPHDGAssemblyHelper(const MooseObject * const moose_obj,
                                     MooseVariableDependencyInterface * const mvdi,
                                     const TransientInterface * const ti,
                                     SystemBase & sys,
                                     const Assembly & assembly,
                                     const THREAD_ID tid,
                                     const std::set<SubdomainID> & block_ids,
                                     const std::set<BoundaryID> & boundary_ids);

  virtual void scalarVolume() override {}

  virtual void scalarFace() override;

  virtual void lmFace() override;

  virtual void scalarDirichlet(const Moose::Functor<Real> &) override
  {
    mooseError("A Dirichlet method doesn't make sense for this assembly helper");
  }

  const MooseVariableField<Real> & _vel_x_var;
  const MooseVariableField<Real> & _vel_y_var;
  const ADVariableValue & _vel_x;
  const ADVariableValue & _vel_y;
  const unsigned short _comp;
  const Real _gamma;
  const Moose::Functor<ADRealVectorValue> & _face_velocity;
  /// Facet characteristic length for correct norm computations
  Real _hmax;

private:
  ADReal computeQpResidualOnSide(const unsigned int qp);
};
