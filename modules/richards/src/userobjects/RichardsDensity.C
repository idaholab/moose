/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

//  Fluid density base class.
//
#include "RichardsDensity.h"

template <>
InputParameters
validParams<RichardsDensity>()
{
  InputParameters params = validParams<GeneralUserObject>();
  params.addClassDescription(
      "Fluid density base class.  Override density, ddensity and d2density in your class");
  return params;
}

RichardsDensity::RichardsDensity(const InputParameters & parameters) : GeneralUserObject(parameters)
{
}

void
RichardsDensity::initialize()
{
}

void
RichardsDensity::execute()
{
}

void
RichardsDensity::finalize()
{
}
