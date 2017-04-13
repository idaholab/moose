/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "BCUserObject.h"

template <>
InputParameters
validParams<BCUserObject>()
{
  InputParameters params = validParams<GeneralUserObject>();
  return params;
}

BCUserObject::BCUserObject(const InputParameters & parameters) : GeneralUserObject(parameters) {}

void
BCUserObject::initialize()
{
}

void
BCUserObject::execute()
{
}

void
BCUserObject::finalize()
{
}

void
BCUserObject::threadJoin(const UserObject & /*y*/)
{
}
