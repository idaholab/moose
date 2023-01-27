//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RenamedMatReaction.h"

registerMooseObject("MooseTestApp", RenamedMatReaction);

InputParameters
RenamedMatReaction::validParams()
{
  InputParameters params = MatReaction::validParams();
  params.renameParam("mob_name", "reaction_coefficient", "The reaction rate used with the kernel");
  return params;
}

RenamedMatReaction::RenamedMatReaction(const InputParameters & parameters) : MatReaction(parameters)
{
}
