//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Residual.h"

#include "FEProblem.h"
#include "SubProblem.h"
#include "NonlinearSystem.h"

registerMooseObject("MooseApp", Residual);

InputParameters
Residual::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();
  params.addClassDescription("Report the non-linear residual.");
  MooseEnum residual_types(
      "FINAL INITIAL_BEFORE_PRESET INITIAL_AFTER_PRESET PRE_SMO INITIAL CURRENT", "FINAL");
  params.addParam<MooseEnum>("residual_type", residual_types, "Type of residual to be reported.");
  return params;
}

Residual::Residual(const InputParameters & parameters)
  : GeneralPostprocessor(parameters), _residual_type(getParam<MooseEnum>("residual_type"))
{
  if (_residual_type == "INITIAL_BEFORE_PRESET")
    mooseDeprecated("INITIAL_BEFORE_PRESET is deprecated, use PRE_SMO instead.");
  if (_residual_type == "INITIAL_AFTER_PRESET")
    mooseDeprecated("INITIAL_AFTER_PRESET is deprecated, use INITIAL instead.");
}

Real
Residual::getValue() const
{
  Real residual = 0.0;
  if (_residual_type == "FINAL")
    residual = _subproblem.finalNonlinearResidual(_sys.number());
  else if (_residual_type == "CURRENT")
  {
    const auto & snes = _fe_problem.getNonlinearSystemBase(_sys.number()).getSNES();

    PetscReal norm;
    LibmeshPetscCall(SNESGetFunctionNorm(snes, &norm));

    residual = norm;
  }
  else
  {
    FEProblemBase * fe_problem = dynamic_cast<FEProblemBase *>(&_subproblem);
    if (!fe_problem)
      mooseError("Dynamic cast to FEProblemBase failed in Residual Postprocessor");

    const auto & nl_sys = fe_problem->getNonlinearSystemBase(_sys.number());

    if (_residual_type == "INITIAL_BEFORE_PRESET" || _residual_type == "PRE_SMO")
      residual = nl_sys.preSMOResidual();
    else if (_residual_type == "INITIAL_AFTER_PRESET" || _residual_type == "INITIAL")
      residual = nl_sys.initialResidual();
    else
      mooseError("Invalid residual_type option in Residual Postprocessor: ", _residual_type);
  }
  return residual;
}
