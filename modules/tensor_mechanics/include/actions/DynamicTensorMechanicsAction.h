//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "TensorMechanicsAction.h"

class DynamicTensorMechanicsAction : public TensorMechanicsAction
{
public:
  static InputParameters validParams();

  DynamicTensorMechanicsAction(const InputParameters & params);

  virtual void act() override;

protected:
  virtual std::string getKernelType() override;
  virtual InputParameters getKernelParameters(std::string type) override;

  std::vector<AuxVariableName> _velocities;
  std::vector<AuxVariableName> _accelerations;

  const Real _newmark_beta;
  const Real _newmark_gamma;
  const Real _hht_alpha;
};
