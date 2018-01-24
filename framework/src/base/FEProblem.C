/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

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
