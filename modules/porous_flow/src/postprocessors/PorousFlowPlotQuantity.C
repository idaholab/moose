/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/


#include "PorousFlowPlotQuantity.h"
#include "PorousFlowSumQuantity.h"

template<>
InputParameters validParams<PorousFlowPlotQuantity>()
{
  InputParameters params = validParams<GeneralPostprocessor>();
  params.addRequiredParam<UserObjectName>("uo", "PorousFlowSumQuantity user object name that holds the required information");

  return params;
}

PorousFlowPlotQuantity::PorousFlowPlotQuantity(const InputParameters & parameters) :
    GeneralPostprocessor(parameters),
    _total_mass(getUserObject<PorousFlowSumQuantity>("uo"))
{
}

PorousFlowPlotQuantity::~PorousFlowPlotQuantity()
{
}

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

