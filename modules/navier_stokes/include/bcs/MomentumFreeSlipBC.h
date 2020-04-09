//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "NodalNormalBC.h"

/**
 * Boundary condition that applies free slip condition at nodes
 */
class MomentumFreeSlipBC : public NodalNormalBC
{
public:
  static InputParameters validParams();

  MomentumFreeSlipBC(const InputParameters & parameters);
  virtual ~MomentumFreeSlipBC();

  virtual bool shouldApply() override;

protected:
  virtual Real computeQpResidual() override;

  /// The dimension of the mesh
  const unsigned int _mesh_dimension;

  /// Momentum in x-direction
  const VariableValue & _rho_u;
  /// Momentum in y-direction
  const VariableValue & _rho_v;
  /// Momentum in z-direction
  const VariableValue & _rho_w;
};
