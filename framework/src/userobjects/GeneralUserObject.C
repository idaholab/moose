//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GeneralUserObject.h"

InputParameters
GeneralUserObject::validParams()
{
  InputParameters params = UserObject::validParams();
  params += MaterialPropertyInterface::validParams();

  return params;
}

GeneralUserObject::GeneralUserObject(const InputParameters & parameters)
  : UserObject(parameters),
    ScalarCoupleable(this),
    MaterialPropertyInterface(this, Moose::EMPTY_BLOCK_IDS, Moose::EMPTY_BOUNDARY_IDS),
    TransientInterface(this)
{
}

void
GeneralUserObject::threadJoin(const UserObject &)
{
  mooseError("GeneralUserObjects do not execute using threads, this function does nothing and "
             "should not be used.");
}

void
GeneralUserObject::subdomainSetup()
{
  mooseError("GeneralUserObjects do not execute subdomainSetup method, this function does nothing "
             "and should not be used.");
}
