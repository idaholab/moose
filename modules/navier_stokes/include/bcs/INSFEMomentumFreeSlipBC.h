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
 * Boundary condition for free slip wall boundary
 * This implementation follows the MomentumFreeSlipBC of the Navier-Stokes module,
 * which acts on conservative variables such as rhou and rhov, while this class acts on
 * primitive variables such as u and v.
 */
class INSFEMomentumFreeSlipBC : public NodalNormalBC
{
public:
  static InputParameters validParams();

  INSFEMomentumFreeSlipBC(const InputParameters & parameters);
  virtual ~INSFEMomentumFreeSlipBC();

  virtual bool shouldApply() override;

protected:
  virtual Real computeQpResidual() override;

  /// The dimension of the mesh
  const unsigned int _mesh_dimension;

  const VariableValue & _u_vel;
  const VariableValue & _v_vel;
  const VariableValue & _w_vel;
};
