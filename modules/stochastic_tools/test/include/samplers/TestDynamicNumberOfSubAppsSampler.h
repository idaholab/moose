//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MonteCarloSampler.h"

class TestDynamicNumberOfSubAppsSampler : public MonteCarloSampler
{
public:
  static InputParameters validParams();
  TestDynamicNumberOfSubAppsSampler(const InputParameters & parameters);
  void executeSetUp() override;

protected:
  const dof_id_type _increment_rows;
};
