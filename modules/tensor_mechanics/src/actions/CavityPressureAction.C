/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "CavityPressureAction.h"

#include "Factory.h"
#include "FEProblem.h"
#include "Parser.h"
#include "Conversion.h"

template<>
InputParameters validParams<CavityPressureAction>()
{
  InputParameters params = validParams<Action>();
  params.addRequiredParam<std::vector<BoundaryName> >("boundary", "The list of boundary IDs from the mesh where the pressure will be applied");
  params.addRequiredParam<std::vector<NonlinearVariableName> >("displacements", "The nonlinear displacement variables");
  params.addParam<std::vector<AuxVariableName> >("save_in", "Auxiliary variables to save the displacement residuals");
  params.addParam<std::string>("output", "The name to use for the plenum pressure value");
  params.addParam<bool>("use_displaced_mesh", false, "Whether to use displaced mesh in the boundary condition");
  return params;
}

CavityPressureAction::CavityPressureAction(const InputParameters & params) :
  Action(params),
  _boundary(getParam<std::vector<BoundaryName> >("boundary"))
{
}

void
CavityPressureAction::act()
{
  std::vector<NonlinearVariableName> displacements = getParam<std::vector<NonlinearVariableName> > ("displacements");
  unsigned int _ndisp = displacements.size();

  std::vector<AuxVariableName> this_save_in = getParam<std::vector<AuxVariableName> >("save_in");
  if (isParamValid("save_in") && this_save_in.size() != _ndisp)
    mooseError("Number of save_in variables should equal to the number of displacement variables: " << _ndisp);

  std::vector<std::vector<AuxVariableName> > save_in(_ndisp);
  if (isParamValid("save_in"))
    for (unsigned int i = 0; i < _ndisp; ++i)
      save_in[i].push_back(this_save_in[i]);

  PostprocessorName ppname;
  if (isParamValid("output"))
    ppname = getParam<std::string>("output");
  else
    ppname = _name;

  InputParameters params = _factory.getValidParams("Pressure");
  params.set<std::vector<BoundaryName> >("boundary") = _boundary;
  params.set<PostprocessorName>("postprocessor") = ppname;
  params.set<bool>("use_displaced_mesh") = getParam<bool>("use_displaced_mesh");

  for (unsigned int i = 0; i < _ndisp; ++i)
  {
    std::string name = _name + "_" + Moose::stringify(i);

    params.set<unsigned int>("component") = i;
    params.set<NonlinearVariableName>("variable") = displacements[i];
    params.set<std::vector<AuxVariableName> >("save_in") = save_in[i];

    _problem->addBoundaryCondition("Pressure", name, params);
  }
}
