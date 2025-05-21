//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
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
 * contribution proportional to the jump in velocity at an internal face. This can be done in order
 * to produce an augmented Lagrange like perturbation for DG discretizations. Obviously there is no
 * jump at an external face, but we must have this object anyway in order to compute a consistent
 * term diagonal for both internal and external faces
 */
class MassFluxPenaltyBC : public ADIntegratedBC
{
public:
  static InputParameters validParams();

  MassFluxPenaltyBC(const InputParameters & parameters);

  virtual void computeResidual() override;

protected:
  virtual ADReal computeQpResidual() override;

  const ADVariableValue & _vel_x;
  const ADVariableValue & _vel_y;
  const unsigned short _comp;
  /// whether to avoid contributing to the residual
  const bool _matrix_only;
  /// Stabilization magnitude parameter
  const Real _gamma;
};
