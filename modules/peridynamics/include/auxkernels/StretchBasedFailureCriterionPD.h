//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "BondStatusBasePD.h"

/**
 * Bond stretch based failure ctriterion to update the bond status for fracture modeling
 */
class StretchBasedFailureCriterionPD : public BondStatusBasePD
{
public:
  static InputParameters validParams();

  StretchBasedFailureCriterionPD(const InputParameters & parameters);

protected:
  virtual Real computeFailureCriterionValue() override;

  /// Material property containing the mechanical stretch
  const MaterialProperty<Real> & _mechanical_stretch;
};
