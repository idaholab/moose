//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MechanicsBaseNOSPD.h"

/**
 * Kernel class for weak plane stress formulation based on Form I of the horizon-stabilized
 * peridynamic correspondence model
 */
class WeakPlaneStressNOSPD : public MechanicsBaseNOSPD
{
public:
  static InputParameters validParams();

  WeakPlaneStressNOSPD(const InputParameters & parameters);

  virtual void computeLocalResidual() override;
  virtual void computeLocalJacobian() override;
  virtual void computeLocalOffDiagJacobian(unsigned int jvar_num,
                                           unsigned int coupled_component) override;
  virtual void computePDNonlocalOffDiagJacobian(unsigned int jvar_num,
                                                unsigned int coupled_component) override;
};
