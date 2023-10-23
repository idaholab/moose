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
#include "TimeDerivative.h"
#include "Material.h"

// Forward Declarations

/**
 * A class for defining the time derivative of the heat equation.
 *
 * By default this Kernel computes:
 *   \f$ \rho * c_p * \frac{\partial T}{\partial t}, \f$
 * where \f$ \rho \f$ and \f$ c_p \f$ are material properties with the names "density" and
 * "specific_heat", respectively.
 */
class HeatConductionTimeDerivative : public TimeDerivative
{
public:
  /// Contructor for Heat Equation time derivative term.
  static InputParameters validParams();

  HeatConductionTimeDerivative(const InputParameters & parameters);

protected:
  /// Compute the residual of the Heat Equation time derivative.
  virtual Real computeQpResidual();

  /// Compute the jacobian of the Heat Equation time derivative.
  virtual Real computeQpJacobian();

  const MaterialProperty<Real> & _specific_heat;
  const MaterialProperty<Real> & _density;
};
