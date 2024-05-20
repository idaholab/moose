//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "LinearFVElementalKernel.h"

/**
 * Kernel that adds the component of the pressure gradient in the momentum
 * equations to the right hand side.
 */
class LinearFVMomentumPressure : public LinearFVElementalKernel
{
public:
  static InputParameters validParams();

  /**
   * Class constructor.
   * @param params The InputParameters for the kernel.
   */
  LinearFVMomentumPressure(const InputParameters & params);

  virtual Real computeMatrixContribution() override;

  virtual Real computeRightHandSideContribution() override;

protected:
  MooseLinearVariableFV<Real> & getPressureVariable(const std::string & vname);

  /// Index x|y|z of the momentum equation component
  const unsigned int _index;

  /// Pointer to the linear finite volume pressure variable
  MooseLinearVariableFV<Real> & _pressure_var;

  /// The pressure variable
  const std::vector<std::unique_ptr<NumericVector<Number>>> & _pressure_gradient;

  /// Cache for the pressure variable number
  const unsigned int _pressure_var_num;

  /// Cache for the pressure system number
  const unsigned int _pressure_sys_num;
};
