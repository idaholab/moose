//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "NodalBC.h"

// Forward Declarations
class IdealGasFluidProperties;

/**
 * This class is used on a boundary where the incoming flow
 * values (rho, u, v, T) are all completely specified.
 */
class NSInflowThermalBC : public NodalBC
{
public:
  static InputParameters validParams();

  NSInflowThermalBC(const InputParameters & parameters);

protected:
  // In general, the residual equation is u-u_d=0, where u_d
  // is a Dirichlet value.  Note that no computeQpJacobian()
  // function can be specified in this class... it is assumed
  // to simply have a 1 on the diagonal.
  virtual Real computeQpResidual();

  // The specified density for this inflow boundary
  const Real _specified_rho;

  // The specified temperature for this inflow boundary
  const Real _specified_temperature;

  // The specified velocity magnitude for this inflow boundary
  const Real _specified_velocity_magnitude;

  // Fluid properties
  const IdealGasFluidProperties & _fp;
};
