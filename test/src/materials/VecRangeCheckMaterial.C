//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "VecRangeCheckMaterial.h"

registerMooseObject("MooseTestApp", VecRangeCheckMaterial);

InputParameters
VecRangeCheckMaterial::validParams()
{
  InputParameters params = Material::validParams();
  params.addRequiredRangeCheckedParam<std::vector<Real>>(
      "rv3", "rv3_size = 3", "Real vector of length 3");
  params.addRequiredRangeCheckedParam<std::vector<int>>(
      "iv3", "iv3_size = 3", "Int vector of length 3");
  params.addRequiredRangeCheckedParam<std::vector<Real>>(
      "rvp", "rvp > 0", "Real vector of all positive values");
  params.addRequiredRangeCheckedParam<std::vector<unsigned int>>(
      "uvg", "uvg_0 > uvg_1", "Unsigned int vector where component 0 is bigger than component 1");
  params.addRequiredRangeCheckedParam<std::vector<long>>(
      "lvg", "lvg_0 > lvg_1", "Long vector where component 0 is bigger than component 1");
  params.addRequiredRangeCheckedParam<std::vector<int>>(
      "ivg", "ivg_0 > ivg_1", "Int vector where component 0 is bigger than component 1");
  params.addRequiredRangeCheckedParam<std::vector<Real>>(
      "rvg", "rvg_0 > rvg_1", "Real vector where component 0 is bigger than component 1");
  params.addRequiredRangeCheckedParam<std::vector<Real>>(
      "rvl",
      "rvl_10 > 0",
      "Testing if component 10 is positive (usually we should have a size check here as well)");
  params.addRequiredRangeCheckedParam<std::vector<Real>>(
      "rve", "rve_size = 0 | rve_size = 3", "Vector with either 0 or 3 components");
  return params;
}

VecRangeCheckMaterial::VecRangeCheckMaterial(const InputParameters & parameters)
  : Material(parameters)
{
}

void
VecRangeCheckMaterial::computeQpProperties()
{
}
