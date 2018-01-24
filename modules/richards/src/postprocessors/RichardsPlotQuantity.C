/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "RichardsPlotQuantity.h"
#include "RichardsSumQuantity.h"

template <>
InputParameters
validParams<RichardsPlotQuantity>()
{
  InputParameters params = validParams<GeneralPostprocessor>();
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
