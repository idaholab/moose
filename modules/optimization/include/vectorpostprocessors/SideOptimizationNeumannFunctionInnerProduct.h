//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SideOptimizationFunctionInnerProduct.h"

class SideOptimizationNeumannFunctionInnerProduct : public SideOptimizationFunctionInnerProduct
{
public:
  static InputParameters validParams();
  SideOptimizationNeumannFunctionInnerProduct(const InputParameters & parameters);

protected:
  virtual Real computeQpInnerProduct() override;
};
