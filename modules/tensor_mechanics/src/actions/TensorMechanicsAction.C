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
#include "Conversion.h"

template<>
InputParameters validParams<TensorMechanicsAction>()
{
  InputParameters params = validParams<Action>();
  params.addClassDescription("Set up stress divergence kernels");
  params.addParam<std::vector<NonlinearVariableName> >("displacements", "The nonlinear displacement variables for the problem");
  params.addParam<NonlinearVariableName>("disp_x", "The x displacement");  // depricated
  params.addParam<NonlinearVariableName>("disp_y", "The y displacement");  // depricated
  params.addParam<NonlinearVariableName>("disp_z", "The z displacement");  // depricated
  params.addParam<NonlinearVariableName>("temp", "The temperature");
  params.addParam<std::string>("base_name", "Material property base name");
  params.addParam<bool>("use_displaced_mesh", false, "Whether to use displaced mesh in the kernels");
  params.addParam<std::vector<SubdomainName> >("block", "The list of ids of the blocks (subdomain) that the stress divergence kernel will be applied to");
  params.addParam<std::vector<AuxVariableName> >("save_in_disp_x", "Auxiliary variables to save the x displacement residuals.");
  params.addParam<std::vector<AuxVariableName> >("save_in_disp_y", "Auxiliary variables to save the y displacement residuals.");
  params.addParam<std::vector<AuxVariableName> >("save_in_disp_z", "Auxiliary variables to save the z displacement residuals.");

  return params;
}

TensorMechanicsAction::TensorMechanicsAction(const InputParameters & params) :
    Action(params)
{
}

void
TensorMechanicsAction::act()
{
  std::vector<NonlinearVariableName> displacements;
  //This code will be the up-to-date version using the displacement string
  if (isParamValid("displacements"))
    displacements = getParam<std::vector<NonlinearVariableName> > ("displacements");

  //This code is for the depricated version that allows three unique displacement variables
  else if (isParamValid("disp_x"))
  {
    mooseDeprecated("StressDivergenceTensors has been updated to accept a string of displacement variable names, e.g. displacements = 'disp_x disp_y' in the input file.");
    displacements.push_back(getParam<NonlinearVariableName>("disp_x"));
    if (isParamValid("disp_y"))
    {
      displacements.push_back(getParam<NonlinearVariableName>("disp_y"));
      if (isParamValid("disp_z"))
         displacements.push_back(getParam<NonlinearVariableName>("disp_z"));
    }
  }

  else
    mooseError("The input file should specify a string of displacement names; these names should match the Variable block names.");

  std::vector<VariableName> coupled_displacements;  //vector to hold the string of coupled disp variables
  unsigned int dim = displacements.size();

  for (unsigned int i = 0; i < dim; ++i)
    coupled_displacements.push_back(displacements[i]);


  // Retain this code 'as is' because StressDivergenceTensors inherits from Kernel.C
  std::vector<std::vector<AuxVariableName> > save_in(dim);

  if (isParamValid("save_in_disp_x"))
    save_in[0] = getParam<std::vector<AuxVariableName> >("save_in_disp_x");

  if (isParamValid("save_in_disp_y"))
    save_in[1] = getParam<std::vector<AuxVariableName> >("save_in_disp_y");

  if (isParamValid("save_in_disp_z"))
    save_in[2] = getParam<std::vector<AuxVariableName> >("save_in_disp_z");


  InputParameters params = _factory.getValidParams("StressDivergenceTensors");
  params.set<std::vector<VariableName> >("displacements") = coupled_displacements;

  if (isParamValid("temp"))
    params.addCoupledVar("temp", "");

  params.set<bool>("use_displaced_mesh") = getParam<bool>("use_displaced_mesh");

  if (isParamValid("base_name"))
    params.set<std::string>("base_name") = getParam<std::string>("base_name");

// Check whether this StressDivergenceTensor kernel is restricted to certain block?
  if (isParamValid("block"))
    params.set<std::vector<SubdomainName> >("block") = getParam<std::vector<SubdomainName> >("block");

  for (unsigned int i = 0; i < dim; ++i)
  {
    std::string kernel_name = "TensorMechanics_" + Moose::stringify(i);

    params.set<unsigned int>("component") = i;
    params.set<NonlinearVariableName>("variable") = displacements[i];
    params.set<std::vector<AuxVariableName> >("save_in") = save_in[i];

    addkernel(kernel_name, params);
  }
}

void
TensorMechanicsAction::addkernel(const std::string & name,  InputParameters & params)
{
  _problem->addKernel("StressDivergenceTensors", name, params);
}
