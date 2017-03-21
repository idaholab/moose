/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

//  Base class for relative permeability as a function of effective saturation
//
#include "RichardsRelPerm.h"

template <>
InputParameters
validParams<RichardsRelPerm>()
{
  InputParameters params = validParams<GeneralUserObject>();
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
