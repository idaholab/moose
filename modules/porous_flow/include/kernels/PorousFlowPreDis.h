//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef POROUSFLOWPREDIS_H
#define POROUSFLOWPREDIS_H

#include "TimeDerivative.h"
#include "PorousFlowDictator.h"

// Forward Declarations
class PorousFlowPreDis;

template <>
InputParameters validParams<PorousFlowPreDis>();

/**
 * Kernel = sum (stoichiometry * density * porosity_old * reaction_rate)
 * where the sum is over secondary chemical species in
 * a precipitation-dissolution reaction system.
 */
class PorousFlowPreDis : public TimeKernel
{
public:
  PorousFlowPreDis(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

  /// Density of the mineral species
  const std::vector<Real> _mineral_density;

  /// holds info on the PorousFlow variables
  const PorousFlowDictator & _dictator;

  /// old value of porosity
  const MaterialProperty<Real> & _porosity_old;

  /// reaction rate of the yielding the secondary species
  const MaterialProperty<std::vector<Real>> & _reaction_rate;

  /// d(reaction rate)/d(porflow variable)
  const MaterialProperty<std::vector<std::vector<Real>>> & _dreaction_rate_dvar;

  /// stoichiometric coefficients
  const std::vector<Real> _stoichiometry;

  /**
   * Derivative of residual with respect to PorousFlow variable number pvar
   * This is used by both computeQpJacobian and computeQpOffDiagJacobian
   * @param pvar take the derivative of the residual wrt this PorousFlow variable
   */
  Real computeQpJac(unsigned int pvar);
};

#endif // POROUSFLOWPREDIS_H
