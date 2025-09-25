//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADIntegratedBC.h"

/*
 * The external face pair to the MassFluxPenalty DGKernel. The DGKernel adds a residual/Jacobian
 * contribution proportional to the jump in velocity at an internal face. This object running on the
 * external boundary adds a residual proportional to the jump between the finite element solution
 * and the Dirichlet value (which this object currently requires). These jump terms are added in
 * order to produce an augmented Lagrange like perturbation for DG discretizations
 */
class MassFluxPenaltyBC : public ADIntegratedBC
{
public:
  static InputParameters validParams();

  MassFluxPenaltyBC(const InputParameters & parameters);

  virtual void computeResidual() override;

protected:
  void precalculateResidual() override;
  virtual ADReal computeQpResidual() override;

  const ADVariableValue & _vel_x;
  const ADVariableValue & _vel_y;
  const unsigned short _comp;
  /// whether to avoid contributing to the residual
  const bool _matrix_only;
  /// Stabilization magnitude parameter
  const Real _gamma;
  /// The velocity value on the boundary. For a Dirichlet boundary this should be the Dirichlet
  /// value. For a Neumann boundary this should be the trace velocity
  const Moose::Functor<ADRealVectorValue> & _face_functor;

  /// Facet characteristic length for correct norm computations
  Real _hmax;
};
