//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MooseTestProblem.h"
#include "AuxiliarySystem.h"

#include "libmesh/system.h"
#include "libmesh/fe_type.h"

registerMooseObject("MooseTestApp", MooseTestProblem);

InputParameters
MooseTestProblem::validParams()
{
  InputParameters params = FEProblem::validParams();
  return params;
}

MooseTestProblem::MooseTestProblem(const InputParameters & params) : FEProblem(params)
{
  _console << "Hello, I am your FEProblemBase-derived class with coordinate type "
           << getParam<MultiMooseEnum>("coord_type") << " and my name is '" << this->name() << "'"
           << std::endl;

  _test_aux = std::make_shared<AuxiliarySystem>(*this, "aux1");
  _test_aux->system().add_variable("dummy", FEType());
}

MooseTestProblem::~MooseTestProblem() { _console << "Goodbye!" << std::endl; }
