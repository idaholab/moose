//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FEProblem.h"

#include "Assembly.h"
#include "AuxiliarySystem.h"
#include "MooseEigenSystem.h"
#include "NonlinearSystem.h"

template <>
InputParameters
validParams<FEProblem>()
{
  InputParameters params = validParams<FEProblemBase>();
  return params;
}

FEProblem::FEProblem(const InputParameters & parameters)
  : FEProblemBase(parameters),
    _use_nonlinear(getParam<bool>("use_nonlinear")),
    _nl_sys(_use_nonlinear ? (std::make_shared<NonlinearSystem>(*this, "nl0"))
                           : (std::make_shared<MooseEigenSystem>(*this, "eigen0")))
{
  _nl = _nl_sys;
  _aux = std::make_shared<AuxiliarySystem>(*this, "aux0");

  newAssemblyArray(*_nl_sys);

  initNullSpaceVectors(parameters, *_nl_sys);

  _eq.parameters.set<FEProblem *>("_fe_problem") = this;
}

FEProblem::~FEProblem() { FEProblemBase::deleteAssemblyArray(); }

void
FEProblem::setInputParametersFEProblem(InputParameters & parameters)
{
  // set _fe_problem
  FEProblemBase::setInputParametersFEProblem(parameters);
  // set _fe_problem
  parameters.set<FEProblem *>("_fe_problem") = this;
}
