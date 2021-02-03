//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html
#pragma once

#include "Sampler.h"

class TestSampler : public Sampler
{
public:
  static InputParameters validParams();
  TestSampler(const InputParameters & parameters);
  virtual void executeSetUp() override;

protected:
  virtual Real computeSample(dof_id_type row_index, dof_id_type col_index) override;

private:
  const bool _use_rand;
  const MooseEnum _error_test;
};
