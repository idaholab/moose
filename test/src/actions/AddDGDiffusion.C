//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AddDGDiffusion.h"
#include "Factory.h"
#include "InputParameters.h"
#include "FEProblemBase.h"

registerMooseAction("MooseTestApp", AddDGDiffusion, "add_dg_kernel");

template <>
InputParameters
validParams<AddDGDiffusion>()
{
  auto params = validParams<Action>();
  params.addRequiredParam<NonlinearVariableName>("variable",
                                                 "The variable on which the dgkernel will act");
  params.addRequiredParam<Real>("sigma", "sigma");
  params.addRequiredParam<Real>("epsilon", "epsilon");
  params.addParam<MaterialPropertyName>(
      "diff", "The diffusion (or thermal conductivity or viscosity) coefficient.");

  return params;
}

AddDGDiffusion::AddDGDiffusion(const InputParameters & params) : Action(params) {}

void
AddDGDiffusion::act()
{
  auto params = _factory.getValidParams("DGDiffusion");
  params.set<NonlinearVariableName>("variable") = _pars.get<NonlinearVariableName>("variable");
  params.set<Real>("sigma") = _pars.get<Real>("sigma");
  params.set<Real>("epsilon") = _pars.get<Real>("epsilon");
  if (_pars.isParamValid("diff"))
    params.set<MaterialPropertyName>("diff") = _pars.get<MaterialPropertyName>("diff");
  _problem->addDGKernel("DGDiffusion", "dg_diffusion", params);
}
