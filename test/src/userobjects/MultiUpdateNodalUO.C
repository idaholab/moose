//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MultiUpdateNodalUO.h"

registerMooseObject("MooseTestApp", MultiUpdateNodalUO);

InputParameters
MultiUpdateNodalUO::validParams()
{
  InputParameters params = NodalUserObject::validParams();
  params.addClassDescription("Test object for the writableVariable API for nodal values");
  params.addRequiredCoupledVar("v", "Coupled variable that will be written to by the test object.");
  return params;
}

MultiUpdateNodalUO::MultiUpdateNodalUO(const InputParameters & parameters)
  : NodalUserObject(parameters), _v(writableVariable("v"))
{
}

void
MultiUpdateNodalUO::execute()
{
  _v.setNodalValue(1.23);
}
