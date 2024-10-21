//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "VecMultiMooseEnumMaterial.h"

registerMooseObject("MooseTestApp", VecMultiMooseEnumMaterial);

InputParameters
VecMultiMooseEnumMaterial::validParams()
{
  InputParameters params = Material::validParams();
  std::vector<MultiMooseEnum> mme = {MultiMooseEnum("a b c d e")};
  params.addRequiredParam<std::vector<MultiMooseEnum>>("mme", mme, "Vector of MultiMooseEnums");
  return params;
}

VecMultiMooseEnumMaterial::VecMultiMooseEnumMaterial(const InputParameters & parameters)
  : Material(parameters)
{
  const std::vector<MultiMooseEnum> & vmme = getParam<std::vector<MultiMooseEnum>>("mme");
  Moose::out << "mme: ";
  for (const auto & i : vmme)
    Moose::out << i << "; ";
  Moose::out << std::endl;
}

void
VecMultiMooseEnumMaterial::computeQpProperties()
{
}
