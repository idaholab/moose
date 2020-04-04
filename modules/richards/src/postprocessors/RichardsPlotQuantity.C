//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RichardsPlotQuantity.h"
#include "RichardsSumQuantity.h"

registerMooseObject("RichardsApp", RichardsPlotQuantity);

InputParameters
RichardsPlotQuantity::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();
  params.addRequiredParam<UserObjectName>("uo", "user object name that has the total mass value");

  return params;
}

RichardsPlotQuantity::RichardsPlotQuantity(const InputParameters & parameters)
  : GeneralPostprocessor(parameters), _total_mass(getUserObject<RichardsSumQuantity>("uo"))
{
}

RichardsPlotQuantity::~RichardsPlotQuantity() {}

void
RichardsPlotQuantity::initialize()
{
}

void
RichardsPlotQuantity::execute()
{
}

PostprocessorValue
RichardsPlotQuantity::getValue()
{
  return _total_mass.getValue();
}
