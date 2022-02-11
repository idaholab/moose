//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Sampler1DBase.h"

template <>
InputParameters
Sampler1DBase<Real>::validParams()
{
  InputParameters params = GeneralVectorPostprocessor::validParams();
  params += SamplerBase::validParams();
  params += BlockRestrictable::validParams();

  params.addRequiredParam<std::vector<std::string>>(
      "property", "Names of the material properties to be output along a line");

  // This parameter exists in BlockRestrictable, but it is made required here
  // because it is undesirable to use the default, which is to add all blocks.
  params.addRequiredParam<std::vector<SubdomainName>>(
      "block", "The list of block ids (SubdomainID) for which this object will be applied");

  return params;
}
