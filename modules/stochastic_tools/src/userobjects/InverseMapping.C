//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "InverseMapping.h"
#include "SubProblem.h"
#include "Assembly.h"

#include "libmesh/sparse_matrix.h"

InputParameters
InverseMapping::validParams()
{
  InputParameters params = UserObject::validParams();

  params.addRequiredParam<UserObjectName>("surrogate", "Blabla.");
  params.addRequiredParam<UserObjectName>("mapping", "Blabla.");
  params.addRequiredParam<VariableName>("variable", "Blabla.");
  params.addRequiredParam<std::vector<Real>>("parameters", "Blabla");
  params.declareControllable("parameters");

  return params;
}

InverseMapping::InverseMapping(const InputParameters & parameters)
  : UserObject(parameters), _input_parameters(getParam<std::vector<Real>>("parameters"))
{
}

void
InverseMapping::execute()
{
  std::cerr << "Something smart" << std::endl;
}
