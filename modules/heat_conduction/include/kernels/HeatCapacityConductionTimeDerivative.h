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
#include "JvarMapInterface.h"
#include "DerivativeMaterialInterface.h"

// Forward Declarations

/**
 * A class for defining the time derivative of the heat equation.
 *
 * By default this Kernel computes:
 *   \f$ C_p * \frac{\partial T}{\partial t}, \f$
 * where \f$ C_p \f$ is material property for the "heat_capacity".
 */
class HeatCapacityConductionTimeDerivative
  : public DerivativeMaterialInterface<JvarMapKernelInterface<TimeDerivative>>
{
public:
  static InputParameters validParams();

  HeatCapacityConductionTimeDerivative(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

  ///@{ Heat capacity and its derivatives with respect to temperature and other coupled variables.
  const MaterialProperty<Real> & _heat_capacity;
  const MaterialProperty<Real> & _d_heat_capacity_dT;
  std::vector<const MaterialProperty<Real> *> _d_heat_capacity_dargs;
  ///@}
};
