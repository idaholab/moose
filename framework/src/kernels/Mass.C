//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Mass.h"

registerMooseObject("MooseApp", Mass);

InputParameters
Mass::validParams()
{
  InputParameters params = Reaction::validParams();
  params.renameParam("rate", "density", "Optional density for scaling the computed mass.");
  params.set<MultiMooseEnum>("vector_tags") = "";
  params.set<MultiMooseEnum>("matrix_tags") = "";
  params.suppressParameter<MultiMooseEnum>("vector_tags");
  params.set<bool>("matrix_only") = true;
  return params;
}

Mass::Mass(const InputParameters & parameters) : Reaction(parameters) {}
