//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "TimeDerivative.h"
#include "PorousFlowDictator.h"

/**
 * Kernel = (desorped_mass - desorped_mass_old)/dt
 * It is NOT lumped to the nodes
 */
class PorousFlowDesorpedMassTimeDerivative : public TimeKernel
{
public:
  static InputParameters validParams();

  PorousFlowDesorpedMassTimeDerivative(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

  /// PorousFlowDictator UserObject
  const PorousFlowDictator & _dictator;

  /// The MOOSE variable number of the concentration variable
  const unsigned int _conc_var_number;

  /// The concentration variable
  const VariableValue & _conc;

  /// Old value of the concentration variable
  const VariableValue & _conc_old;

  /// Porosity at the qps
  const MaterialProperty<Real> & _porosity;

  /// Old value of porosity
  const MaterialProperty<Real> & _porosity_old;

  /// d(porosity)/d(PorousFlow variable) - these derivatives will be wrt variables at the qps
  const MaterialProperty<std::vector<Real>> & _dporosity_dvar;

  /// d(porosity)/d(grad PorousFlow variable) - these derivatives will be wrt grad(vars) at qps
  const MaterialProperty<std::vector<RealGradient>> & _dporosity_dgradvar;

  /**
   * Derivative of residual with respect to variable number jvar
   * This is used by both computeQpJacobian and computeQpOffDiagJacobian
   * @param jvar take the derivative of the residual wrt this Moose variable
   */
  Real computeQpJac(unsigned int jvar) const;
};
