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

InputParameters
AddDGDiffusion::validParams()
{
  auto params = Action::validParams();
  params.addRequiredParam<NonlinearVariableName>("variable",
                                                 "The variable on which the dgkernel will act");
  params.addParam<Real>("sigma", "sigma");
  params.addParam<Real>("epsilon", "epsilon");
  params.addParam<MaterialPropertyName>(
      "diff", 1, "The diffusion (or thermal conductivity or viscosity) coefficient.");
  params.addParam<VariableName>("coupled_var",
                                "The coupled variable that will determine the diffusion "
                                "rate from the DGCoupledConvection dgkernel");
  MooseEnum kernels_to_add("BOTH REGULAR COUPLED", "BOTH");
  params.addParam<MooseEnum>(
      "kernels_to_add", kernels_to_add, "What dgkernels to add from this action");

  return params;
}

AddDGDiffusion::AddDGDiffusion(const InputParameters & params) : Action(params) {}

void
AddDGDiffusion::act()
{
  auto && kernels_to_add = _pars.get<MooseEnum>("kernels_to_add");

  // add DGDiffusion
  if (kernels_to_add == "BOTH" || kernels_to_add == "REGULAR")
  {
    auto params = _factory.getValidParams("DGDiffusion");
    params.applySpecificParameters(_pars, {"variable", "sigma", "epsilon", "diff"});
    _problem->addDGKernel("DGDiffusion", "dg_diffusion", params);
  }

  // add DGCoupledDiffusion
  if (kernels_to_add == "BOTH" || kernels_to_add == "COUPLED")
  {
    auto params = _factory.getValidParams("DGCoupledDiffusion");
    params.set<NonlinearVariableName>("variable") = _pars.get<NonlinearVariableName>("variable");
    params.set<std::vector<VariableName>>("v") = {_pars.get<VariableName>("coupled_var")};
    _problem->addDGKernel("DGCoupledDiffusion", "dg_coupled_diffusion", params);
  }
}
