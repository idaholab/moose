/********************************************************************/
/*                   DO NOT MODIFY THIS HEADER                      */
/*          Subchannel: Thermal Hydraulics Reactor Analysis         */
/*                                                                  */
/*              (c) 2022 Battelle Energy Alliance, LLC              */
/*                      ALL RIGHTS RESERVED                         */
/*                                                                  */
/*             Prepared by Battelle Energy Alliance, LLC            */
/*               Under Contract No. DE-AC07-05ID14517               */
/*               With the U. S. Department of Energy                */
/*                                                                  */
/*               See COPYRIGHT for full restrictions                */
/********************************************************************/

#include "LiquidWaterInterWrapper1PhaseProblem.h"

registerMooseObject("SubChannelApp", LiquidWaterInterWrapper1PhaseProblem);

InputParameters
LiquidWaterInterWrapper1PhaseProblem::validParams()
{
  InputParameters params = InterWrapper1PhaseProblem::validParams();
  params.addClassDescription(
      "Solver class for water-cooled interwrapper of assemblies in a square-lattice arrangement");
  return params;
}

LiquidWaterInterWrapper1PhaseProblem::LiquidWaterInterWrapper1PhaseProblem(
    const InputParameters & params)
  : InterWrapper1PhaseProblem(params)
{
}

double
LiquidWaterInterWrapper1PhaseProblem::computeFrictionFactor(Real Re)
{
  Real a, b;
  if (Re < 1)
  {
    return 64.0;
  }
  else if (Re >= 1 and Re < 5000)
  {
    a = 64.0;
    b = -1.0;
  }
  else if (Re >= 5000 and Re < 30000)
  {
    a = 0.316;
    b = -0.25;
  }
  else
  {
    a = 0.184;
    b = -0.20;
  }
  return a * std::pow(Re, b);
}
