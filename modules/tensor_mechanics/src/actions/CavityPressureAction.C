//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CavityPressureAction.h"
#include "Factory.h"
#include "FEProblem.h"
#include "Conversion.h"

registerMooseAction("TensorMechanicsApp", CavityPressureAction, "add_bc");

InputParameters
CavityPressureAction::validParams()
{
  InputParameters params = Action::validParams();
  params.addRequiredParam<std::vector<BoundaryName>>(
      "boundary", "The list of boundary IDs from the mesh where the pressure will be applied");
  params.addRequiredParam<std::vector<VariableName>>("displacements",
                                                     "The nonlinear displacement variables");
  params.addParam<std::vector<AuxVariableName>>(
      "save_in", "Auxiliary variables to save the displacement residuals");
  params.addParam<std::string>("output", "The name to use for the cavity pressure value");
  params.addParam<bool>(
      "use_displaced_mesh", true, "Whether to use displaced mesh in the boundary condition");
  params.addParam<bool>("use_automatic_differentiation",
                        false,
                        "Flag to use automatic differentiation (AD) objects when possible");
  params.addClassDescription("Action to setup cavity pressure boundary condition");
  return params;
}

CavityPressureAction::CavityPressureAction(const InputParameters & params)
  : Action(params), _use_ad(getParam<bool>("use_automatic_differentiation"))
{
}

void
CavityPressureAction::act()
{
  auto displacements = getParam<std::vector<VariableName>>("displacements");
  auto save_in = getParam<std::vector<AuxVariableName>>("save_in");

  unsigned int ndisp = displacements.size();
  if (save_in.size() > 0 && save_in.size() != ndisp)
    mooseError("Number of save_in variables should equal to the number of displacement variables ",
               ndisp);

  std::string ad_prepend = "";
  if (_use_ad)
    ad_prepend = "AD";

  std::string kernel_name = ad_prepend + "Pressure";

  InputParameters params = _factory.getValidParams(kernel_name);
  params.applyParameters(parameters());

  params.set<PostprocessorName>("postprocessor") =
      isParamValid("output") ? getParam<std::string>("output") : _name;

  for (unsigned int i = 0; i < ndisp; ++i)
  {
    params.set<NonlinearVariableName>("variable") = displacements[i];
    if (!save_in.empty())
      params.set<std::vector<AuxVariableName>>("save_in") = {save_in[i]};
    std::string unique_kernel_name = _name + "_" + Moose::stringify(i);

    _problem->addBoundaryCondition(kernel_name, unique_kernel_name, params);
  }
}
