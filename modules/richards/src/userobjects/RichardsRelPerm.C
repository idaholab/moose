//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

//  Base class for relative permeability as a function of effective saturation
//
#include "RichardsRelPerm.h"

InputParameters
RichardsRelPerm::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params.addClassDescription(
      "Relative permeability base class.  Override relperm, drelperm and d2relperm in your class");
  return params;
}

RichardsRelPerm::RichardsRelPerm(const InputParameters & parameters) : GeneralUserObject(parameters)
{
}

void
RichardsRelPerm::initialize()
{
}

void
RichardsRelPerm::execute()
{
}

void
RichardsRelPerm::finalize()
{
}
