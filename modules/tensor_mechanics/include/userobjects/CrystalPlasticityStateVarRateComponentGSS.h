//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "CrystalPlasticityStateVarRateComponent.h"

/**
 * Phenomenological constitutive model state variable evolution rate component userobject class.
 */
class CrystalPlasticityStateVarRateComponentGSS : public CrystalPlasticityStateVarRateComponent
{
public:
  static InputParameters validParams();

  CrystalPlasticityStateVarRateComponentGSS(const InputParameters & parameters);

  virtual bool calcStateVariableEvolutionRateComponent(unsigned int qp,
                                                       std::vector<Real> & val) const;

protected:
  const MaterialProperty<std::vector<Real>> & _mat_prop_slip_rate;
  const MaterialProperty<std::vector<Real>> & _mat_prop_state_var;

  /// The hardening parameters in this class are read from .i file. The user can override to read from file.
  FileName _slip_sys_hard_prop_file_name;

  std::vector<Real> _hprops;
};
