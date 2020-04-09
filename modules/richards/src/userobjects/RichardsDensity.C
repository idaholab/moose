//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

//  Fluid density base class.
//
#include "RichardsDensity.h"

InputParameters
RichardsDensity::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
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
