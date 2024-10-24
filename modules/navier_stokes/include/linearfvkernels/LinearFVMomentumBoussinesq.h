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
class LinearFVMomentumBoussinesq : public LinearFVElementalKernel
{
public:
  static InputParameters validParams();

  /**
   * Class constructor.
   * @param params The InputParameters for the kernel.
   */
  LinearFVMomentumBoussinesq(const InputParameters & params);

  virtual Real computeMatrixContribution() override;

  virtual Real computeRightHandSideContribution() override;

protected:
  /// Fluid Temperature
  MooseLinearVariableFV<Real> & getTemperatureVariable(const std::string & vname);

  /// Index x|y|z of the momentum equation component
  const unsigned int _index;
  /// Pointer to the linear finite volume temperature variable
  MooseLinearVariableFV<Real> & _temperature_var;
    /// The gravity vector
  const RealVectorValue _gravity;
  /// The thermal expansion coefficient
  const Moose::Functor<ADReal> & _alpha;
  /// Reference temperature at which the value of _rho was measured
  const Real _ref_temperature;
  /// the density
  const Moose::Functor<ADReal> & _rho;

};
