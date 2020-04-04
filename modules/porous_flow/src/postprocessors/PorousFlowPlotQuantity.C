//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowPlotQuantity.h"
#include "PorousFlowSumQuantity.h"

registerMooseObject("PorousFlowApp", PorousFlowPlotQuantity);

InputParameters
PorousFlowPlotQuantity::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();
  params.addRequiredParam<UserObjectName>(
      "uo", "PorousFlowSumQuantity user object name that holds the required information");
  params.addClassDescription("Extracts the value from the PorousFlowSumQuantity UserObject");
  return params;
}

PorousFlowPlotQuantity::PorousFlowPlotQuantity(const InputParameters & parameters)
  : GeneralPostprocessor(parameters), _total_mass(getUserObject<PorousFlowSumQuantity>("uo"))
{
}

PorousFlowPlotQuantity::~PorousFlowPlotQuantity() {}

void
PorousFlowPlotQuantity::initialize()
{
}

void
PorousFlowPlotQuantity::execute()
{
}

PostprocessorValue
PorousFlowPlotQuantity::getValue()
{
  return _total_mass.getValue();
}
