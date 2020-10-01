//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AuxKernel.h"

class GrainBoundaryVelocity : public AuxKernel
{
public:
  static InputParameters validParams();

  GrainBoundaryVelocity(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;

  const Real _eta_bottom;
  const Real _eta_top;
  const unsigned int _op_num;

  const std::vector<const VariableValue *> _eta;
  const std::vector<const VariableValue *> _deta_dt;
  const std::vector<const VariableGradient *> _grad_eta;

  Real _value;
  Real _new_val;
};
