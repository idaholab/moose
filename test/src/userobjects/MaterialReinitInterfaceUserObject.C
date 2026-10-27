//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details

#include "MaterialReinitInterfaceUserObject.h"

registerMooseObject("MooseTestApp", MaterialReinitInterfaceUserObject);

InputParameters
MaterialReinitInterfaceUserObject::validParams()
{
  InputParameters params = InterfaceUserObject::validParams();
  params.addClassDescription(
      "Interface user object with no material dependencies for selective reinit testing.");
  return params;
}

MaterialReinitInterfaceUserObject::MaterialReinitInterfaceUserObject(
    const InputParameters & parameters)
  : InterfaceUserObject(parameters)
{
}

void
MaterialReinitInterfaceUserObject::finalize()
{
}

void
MaterialReinitInterfaceUserObject::threadJoin(const UserObject & /*uo*/)
{
}
