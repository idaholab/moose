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
 *   \f$ \rho * c_p * \frac{\partial T}{\partial t}, \f$
 * where \f$ \rho \f$ and \f$ c_p \f$ are material properties for "density" and
 * "specific_heat", respectively.
 */
class SpecificHeatConductionTimeDerivative
  : public DerivativeMaterialInterface<JvarMapKernelInterface<TimeDerivative>>
{
public:
  static InputParameters validParams();

  SpecificHeatConductionTimeDerivative(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

  ///@{ Specific heat and its derivatives with respect to temperature and other coupled variables.
  const MaterialProperty<Real> & _specific_heat;
  const MaterialProperty<Real> & _d_specific_heat_dT;
  std::vector<const MaterialProperty<Real> *> _d_specific_heat_dargs;
  ///@}

  ///@{ Density and its derivatives with respect to temperature and other coupled variables.
  const MaterialProperty<Real> & _density;
  const MaterialProperty<Real> & _d_density_dT;
  std::vector<const MaterialProperty<Real> *> _d_density_dargs;
  ///@}
};
