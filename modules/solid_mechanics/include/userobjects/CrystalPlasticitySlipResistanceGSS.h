//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "CrystalPlasticitySlipResistance.h"

/**
 * Phenomenological constitutive model slip resistance userobject class.
 */
class CrystalPlasticitySlipResistanceGSS : public CrystalPlasticitySlipResistance
{
public:
  static InputParameters validParams();

  CrystalPlasticitySlipResistanceGSS(const InputParameters & parameters);

  virtual bool calcSlipResistance(unsigned int qp, std::vector<Real> & val) const;

protected:
  const MaterialProperty<std::vector<Real>> & _mat_prop_state_var;
};
