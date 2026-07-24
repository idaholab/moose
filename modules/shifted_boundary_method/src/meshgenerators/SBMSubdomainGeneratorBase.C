//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SBMSubdomainGeneratorBase.h"

InputParameters
SBMSubdomainGeneratorBase::validParams()
{
  InputParameters params = MeshGenerator::validParams();

  params.addRequiredParam<MeshGeneratorName>("input", "The mesh we want to modify");

  params.addRangeCheckedParam<int>("qrule_order",
                                   9,
                                   "qrule_order >= 0 & qrule_order <= 10",
                                   "Quadrature order used to estimate the active area.");

  return params;
}

SBMSubdomainGeneratorBase::SBMSubdomainGeneratorBase(const InputParameters & parameters)
  : MeshGenerator(parameters),
    _input(getMesh("input")),
    _qrule_order(static_cast<Order>(getParam<int>("qrule_order")))
{
}
