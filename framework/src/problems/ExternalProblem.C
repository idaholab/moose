//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "ExternalProblem.h"
#include "NonlinearSystem.h"
#include "AuxiliarySystem.h"

template <>
InputParameters
validParams<ExternalProblem>()
{
  InputParameters params = validParams<FEProblemBase>();
  params.set<bool>("skip_nl_system_check") = true;

  return params;
}

ExternalProblem::ExternalProblem(const InputParameters & parameters) : FEProblemBase(parameters)
{
  _nl = std::make_shared<NonlinearSystem>(*this, "nl0");
  _aux = std::make_shared<AuxiliarySystem>(*this, "aux0");

  newAssemblyArray(*_nl);
}
