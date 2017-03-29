/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "CavityPressureAction.h"
#include "Factory.h"
#include "FEProblem.h"
#include "Conversion.h"

template <>
InputParameters
validParams<CavityPressureAction>()
{
  InputParameters params = validParams<Action>();
  params.addRequiredParam<std::vector<BoundaryName>>(
      "boundary", "The list of boundary IDs from the mesh where the pressure will be applied");
  params.addParam<std::vector<NonlinearVariableName>>("displacements",
                                                      "The nonlinear displacement variables");
  params.addParam<std::vector<AuxVariableName>>(
      "save_in", "Auxiliary variables to save the displacement residuals");
  params.addParam<std::string>("output", "The name to use for the plenum pressure value");
  params.addParam<bool>(
      "use_displaced_mesh", true, "Whether to use displaced mesh in the boundary condition");
  return params;
}

CavityPressureAction::CavityPressureAction(const InputParameters & params) : Action(params) {}

void
CavityPressureAction::act()
{
  auto displacements = getParam<std::vector<NonlinearVariableName>>("displacements");
  auto save_in = getParam<std::vector<AuxVariableName>>("save_in");

  unsigned int ndisp = displacements.size();
  if (save_in.size() > 0 && save_in.size() != ndisp)
    mooseError("Number of save_in variables should equal to the number of displacement variables ",
               ndisp);

  InputParameters params = _factory.getValidParams("Pressure");
  params.applyParameters(parameters());

  params.set<PostprocessorName>("postprocessor") =
      isParamValid("output") ? getParam<std::string>("output") : _name;

  for (unsigned int i = 0; i < ndisp; ++i)
  {
    params.set<unsigned int>("component") = i;
    params.set<NonlinearVariableName>("variable") = displacements[i];
    if (!save_in.empty())
      params.set<std::vector<AuxVariableName>>("save_in") = {save_in[i]};

    _problem->addBoundaryCondition("Pressure", _name + "_" + Moose::stringify(i), params);
  }
}
