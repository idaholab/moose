//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "OnOffNeumannBC.h"

registerMooseObject("MooseTestApp", OnOffNeumannBC);

InputParameters
OnOffNeumannBC::validParams()
{
  InputParameters params = NeumannBC::validParams();

  return params;
}

OnOffNeumannBC::OnOffNeumannBC(const InputParameters & parameters) : NeumannBC(parameters) {}

bool
OnOffNeumannBC::shouldApply() const
{
  return (_t_step == 1) ? false : true;
}
