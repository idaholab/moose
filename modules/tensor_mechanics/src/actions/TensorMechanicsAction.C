/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "TensorMechanicsAction.h"

#include "Factory.h"
#include "FEProblem.h"
#include "Parser.h"

template<>
InputParameters validParams<TensorMechanicsAction>()
{
  InputParameters params = validParams<Action>();
  params.addClassDescription("Set up stress divergence kernels");
  params.addRequiredParam<NonlinearVariableName>("disp_x", "The x displacement");
  params.addParam<NonlinearVariableName>("disp_y", "The y displacement");
  params.addParam<NonlinearVariableName>("disp_z", "The z displacement");
  params.addParam<NonlinearVariableName>("temp", "The temperature");
  params.addParam<std::string>("base_name", "Material property base name");
  params.addParam<bool>("use_displaced_mesh", false, "Whether to use displaced mesh in the kernels");

  params.addParam<std::vector<AuxVariableName> >("save_in_disp_x", "Auxiliary variables to save the x displacement residuals.");
  params.addParam<std::vector<AuxVariableName> >("save_in_disp_y", "Auxiliary variables to save the y displacement residuals.");
  params.addParam<std::vector<AuxVariableName> >("save_in_disp_z", "Auxiliary variables to save the z displacement residuals.");

  return params;
}

TensorMechanicsAction::TensorMechanicsAction(const std::string & name, InputParameters params) :
    Action(name, params)
{
}

void
TensorMechanicsAction::act()
{
  unsigned int dim = 1;
  std::vector<std::string> keys;
  std::vector<VariableName> vars;
  std::string type("StressDivergenceTensors");

  std::vector<std::vector<AuxVariableName> > save_in;
  //Prepare displacements and set value for dim
  keys.push_back("disp_x");
  vars.push_back(getParam<NonlinearVariableName>("disp_x"));

  if (isParamValid("disp_y"))
  {
    ++dim;
    keys.push_back("disp_y");
    vars.push_back(getParam<NonlinearVariableName>("disp_y"));
    if (isParamValid("disp_z"))
    {
      ++dim;
      keys.push_back("disp_z");
      vars.push_back(getParam<NonlinearVariableName>("disp_z"));
    }
  }

  save_in.resize(dim);

  if (isParamValid("save_in_disp_x"))
    save_in[0] = getParam<std::vector<AuxVariableName> >("save_in_disp_x");

  if (isParamValid("save_in_disp_y"))
    save_in[1] = getParam<std::vector<AuxVariableName> >("save_in_disp_y");

  if (isParamValid("save_in_disp_z"))
    save_in[2] = getParam<std::vector<AuxVariableName> >("save_in_disp_z");

  //Add in the temperature
  unsigned int num_coupled(dim);
  if (isParamValid("temp"))
  {
    ++num_coupled;
    keys.push_back("temp");
    vars.push_back(getParam<NonlinearVariableName>("temp"));
  }

  InputParameters params = _factory.getValidParams(type);
  for (unsigned int j = 0; j < num_coupled; ++j)
  {
    params.addCoupledVar(keys[j], "");
    params.set<std::vector<VariableName> >(keys[j]) = std::vector<VariableName>(1, vars[j]);
  }

  params.set<bool>("use_displaced_mesh") = getParam<bool>("use_displaced_mesh");

  if (isParamValid("base_name"))
    params.set<std::string>("base_name") = getParam<std::string>("base_name");

  std::string short_name = "TensorMechanics";

  for (unsigned int i = 0; i < dim; ++i)
  {
    std::stringstream name;
    name << short_name;
    name << i;

    params.set<unsigned int>("component") = i;
    params.set<NonlinearVariableName>("variable") = vars[i];
    params.set<std::vector<AuxVariableName> >("save_in") = save_in[i];

    _problem->addKernel(type, name.str(), params);
  }
}
