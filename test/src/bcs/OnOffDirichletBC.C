//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "OnOffDirichletBC.h"

registerMooseObject("MooseTestApp", OnOffDirichletBC);

InputParameters
OnOffDirichletBC::validParams()
{
  InputParameters params = DirichletBC::validParams();

  return params;
}

OnOffDirichletBC::OnOffDirichletBC(const InputParameters & parameters) : DirichletBC(parameters) {}

OnOffDirichletBC::~OnOffDirichletBC() {}

bool
OnOffDirichletBC::shouldApply()
{
  return (_t_step == 1) ? true : false;
}
