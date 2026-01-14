//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "WarningUserObject.h"

registerMooseObject("MooseTestApp", WarningUserObject);

InputParameters
WarningUserObject::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  return params;
}

WarningUserObject::WarningUserObject(const InputParameters & params)
  : ThreadedGeneralUserObject(params)
{
  mooseWarning("During construction");
}

void
WarningUserObject::initialSetup()
{
  mooseWarning("During initialSetup");
}

void
WarningUserObject::execute()
{
  mooseWarning("During execution");
}

void
WarningUserObject::finalize()
{
  mooseWarning("During finalize");
}

void
WarningUserObject::threadJoin(const UserObject & /*uo*/)
{
  mooseWarning("During threadJoin");
}
