//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "BlocksMaxDimensionPostprocessor.h"

registerMooseObject("MooseTestApp", BlocksMaxDimensionPostprocessor);

InputParameters
BlocksMaxDimensionPostprocessor::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();
  params.addRequiredParam<std::vector<SubdomainName>>("block", "The list of subdomain names");
  return params;
}

BlocksMaxDimensionPostprocessor::BlocksMaxDimensionPostprocessor(const InputParameters & parameters)
  : GeneralPostprocessor(parameters), _blocks(getParam<std::vector<SubdomainName>>("block"))
{
}

Real
BlocksMaxDimensionPostprocessor::getValue()
{
  return getMooseApp().actionWarehouse().mesh()->getBlocksMaxDimension(_blocks);
}
