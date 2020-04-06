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
 * Kernel = sum (stoichiometry * density * porosity_old * saturation * reaction_rate)
 * where the sum is over secondary chemical species in
 * a precipitation-dissolution reaction system.
 */
class PorousFlowPreDis : public TimeKernel
{
public:
  static InputParameters validParams();

  PorousFlowPreDis(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

  /// Density of the mineral species
  const std::vector<Real> _mineral_density;

  /// PorousFlowDictator UserObject
  const PorousFlowDictator & _dictator;

  /// Aqueous phase number
  const unsigned int _aq_ph;

  /// Old value of porosity
  const MaterialProperty<Real> & _porosity_old;

  /// Saturation
  const MaterialProperty<std::vector<Real>> & _saturation;

  /// d(saturation)/d(PorousFlow var)
  const MaterialProperty<std::vector<std::vector<Real>>> & _dsaturation_dvar;

  /// Reaction rate of the yielding the secondary species
  const MaterialProperty<std::vector<Real>> & _reaction_rate;

  /// d(reaction rate)/d(porflow variable)
  const MaterialProperty<std::vector<std::vector<Real>>> & _dreaction_rate_dvar;

  /// Stoichiometric coefficients
  const std::vector<Real> _stoichiometry;

  /**
   * Derivative of residual with respect to PorousFlow variable number pvar
   * This is used by both computeQpJacobian and computeQpOffDiagJacobian
   * @param pvar take the derivative of the residual wrt this PorousFlow variable
   */
  Real computeQpJac(unsigned int pvar);
};
