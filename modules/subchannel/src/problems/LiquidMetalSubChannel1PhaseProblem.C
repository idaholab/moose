#include "LiquidMetalSubChannel1PhaseProblem.h"

registerMooseObject("SubChannelApp", LiquidMetalSubChannel1PhaseProblem);

InputParameters
LiquidMetalSubChannel1PhaseProblem::validParams()
{
  InputParameters params = SubChannel1PhaseProblemBase::validParams();
  return params;
}

LiquidMetalSubChannel1PhaseProblem::LiquidMetalSubChannel1PhaseProblem(
    const InputParameters & params)
  : SubChannel1PhaseProblemBase(params)
{
}

double
LiquidMetalSubChannel1PhaseProblem::computeFrictionFactor(double /*Re*/)
{
  // FIXME: implement this
  return 0;
}
