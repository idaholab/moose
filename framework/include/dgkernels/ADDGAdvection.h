//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADDGKernel.h"

/**
 * Adds residual/Jacobian contributions for a convection term from internal faces for a
 * discontinuous Galerkin formulation
 */
class ADDGAdvection : public ADDGKernel
{
public:
  static InputParameters validParams();

  ADDGAdvection(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual(Moose::DGResidualType type) override;

  /// The velocity on the element
  const ADMaterialProperty<RealVectorValue> & _velocity;
  /// The velocity on the neighbor
  const ADMaterialProperty<RealVectorValue> & _velocity_neighbor;

  /// The advected quantity value on the element side of the face
  const MooseArray<ADReal> & _adv_quant_elem;
  /// The advected quantity value on the neighbor side of the face
  const MooseArray<ADReal> & _adv_quant_neighbor;
};
