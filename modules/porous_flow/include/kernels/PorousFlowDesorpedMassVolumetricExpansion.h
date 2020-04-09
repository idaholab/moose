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
 * Kernel = desorped_mass * d(volumetric_strain)/dt
 * which is not lumped to the nodes
 */
class PorousFlowDesorpedMassVolumetricExpansion : public TimeKernel
{
public:
  static InputParameters validParams();

  PorousFlowDesorpedMassVolumetricExpansion(const InputParameters & parameters);

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

  /// Porosity
  const MaterialProperty<Real> & _porosity;

  /// d(porosity)/d(PorousFlow variable)
  const MaterialProperty<std::vector<Real>> & _dporosity_dvar;

  /// d(porosity)/d(grad PorousFlow variable)
  const MaterialProperty<std::vector<RealGradient>> & _dporosity_dgradvar;

  /// strain rate
  const MaterialProperty<Real> & _strain_rate_qp;

  /// d(strain rate)/d(PorousFlow variable)
  const MaterialProperty<std::vector<RealGradient>> & _dstrain_rate_qp_dvar;

  /**
   * Derivative of the residual with respect to the Moose variable
   * with variable number jvar.
   * @param jvar take the derivative of the mass part of the residual wrt this variable number
   */
  Real computeQpJac(unsigned int jvar) const;
};
