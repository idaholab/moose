//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "MultiAppFieldTransfer.h"
#include "FEProblemBase.h"
#include "DisplacedProblem.h"
#include "MooseVariableFEBase.h"
#include "MooseMesh.h"
#include "SystemBase.h"
#include "MooseAppCoordTransform.h"
#include "MooseMeshUtils.h"

#include "libmesh/system.h"

InputParameters
MultiAppFieldTransfer::validParams()
{
  InputParameters params = MultiAppTransfer::validParams();
  return params;
}

MultiAppFieldTransfer::MultiAppFieldTransfer(const InputParameters & parameters)
  : MultiAppTransfer(parameters)
{
}

void
MultiAppFieldTransfer::initialSetup()
{
  MultiAppTransfer::initialSetup();

  if (_current_direction == TO_MULTIAPP)
    for (auto & to_var : getToVarNames())
      variableIntegrityCheck(to_var);
  else if (_current_direction == FROM_MULTIAPP)
    for (auto & from_var : getFromVarNames())
      variableIntegrityCheck(from_var);
  else
  {
    for (auto & to_var : getToVarNames())
      variableIntegrityCheck(to_var);
    for (auto & from_var : getFromVarNames())
      variableIntegrityCheck(from_var);
  }
}

EquationSystems &
MultiAppFieldTransfer::getEquationSystem(FEProblemBase & problem, bool use_displaced) const
{
  if (use_displaced)
  {
    if (!problem.getDisplacedProblem())
      mooseError("No displaced problem to provide a displaced equation system");
    return problem.getDisplacedProblem()->es();
  }
  else
    return problem.es();
}
