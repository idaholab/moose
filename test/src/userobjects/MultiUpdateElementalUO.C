//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MultiUpdateElementalUO.h"

registerMooseObject("MooseTestApp", MultiUpdateElementalUO);

InputParameters
MultiUpdateElementalUO::validParams()
{
  InputParameters params = ElementUserObject::validParams();
  params.addClassDescription("Test object for the writableVariable API for elemental values");
  params.addRequiredCoupledVar("v", "Coupled variable that will be written to by the test object.");
  return params;
}

MultiUpdateElementalUO::MultiUpdateElementalUO(const InputParameters & parameters)
  : ElementUserObject(parameters), _v(writableVariable("v"))
{
}

void
MultiUpdateElementalUO::execute()
{
  // it really is a DOF value
  _v.setNodalValue(1.23);
}
