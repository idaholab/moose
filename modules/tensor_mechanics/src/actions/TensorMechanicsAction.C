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
  params.addParam<NonlinearVariableName>("disp_x", "The x displacement");//deprecated
  params.addParam<NonlinearVariableName>("disp_y", "The y displacement");//deprecated
  params.addParam<NonlinearVariableName>("disp_z", "The z displacement");//deprecated
  params.addParam<NonlinearVariableName>("temp", "The temperature");
  params.addParam<std::string>("base_name", "Material property base name");
  params.addParam<bool>("use_displaced_mesh", false, "Whether to use displaced mesh in the kernels");
  params.addParam<std::vector<SubdomainName> >("block", "The list of ids of the blocks (subdomain) that the stress divergence kernel will be applied to");
  params.addParam<std::vector<AuxVariableName> >("save_in", "Auxiliary variables to save the displacement residuals");
  params.addParam<std::vector<AuxVariableName> >("save_in_disp_x", "Auxiliary variables to save the x displacement residuals");//deprecated
  params.addParam<std::vector<AuxVariableName> >("save_in_disp_y", "Auxiliary variables to save the y displacement residuals");//deprecated
  params.addParam<std::vector<AuxVariableName> >("save_in_disp_z", "Auxiliary variables to save the z displacement residuals");//deprecated
  params.addParam<std::vector<AuxVariableName> >("diag_save_in", "Auxiliary variables to save the displacement diagonal preconditioner terms");
  params.addParam<std::vector<AuxVariableName> >("diag_save_in_disp_x", "Auxiliary variables to save the x displacement diagonal preconditioner term");//deprecated
  params.addParam<std::vector<AuxVariableName> >("diag_save_in_disp_y", "Auxiliary variables to save the y displacement diagonal preconditioner term");//deprecated
  params.addParam<std::vector<AuxVariableName> >("diag_save_in_disp_z", "Auxiliary variables to save the z displacement diagonal preconditioner term");//deprecated
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
  if (isParamValid("displacements"))
    displacements = getParam<std::vector<NonlinearVariableName> > ("displacements");
  else if (isParamValid("disp_x"))
  {
    mooseDeprecated("StressDivergenceTensors has been updated to accept a string of displacement variable names, e.g. displacements = 'disp_x disp_y disp_z' in the input file.");
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

  unsigned int _ndisp = displacements.size();
  std::vector<VariableName> coupled_displacements;
  for (unsigned int i = 0; i < _ndisp; ++i)
    coupled_displacements.push_back(displacements[i]);

  std::vector<std::vector<AuxVariableName> > save_in(_ndisp);
  if (isParamValid("save_in"))
  {
    std::vector<AuxVariableName> this_save_in = getParam<std::vector<AuxVariableName> >("save_in");
    for (unsigned int i = 0; i < _ndisp; ++i)
      save_in[i].push_back(this_save_in[i]);
  }
  else if (isParamValid("save_in_disp_x"))
  {
    mooseDeprecated("StressDivergenceTensors has been updated to accept a string of save_in variable names, e.g. save_in = 'save_in_disp_x save_in_disp_y save_in_disp_z' in the input file.");
    save_in[0] = getParam<std::vector<AuxVariableName> >("save_in_disp_x");
  }
  else if (isParamValid("save_in_disp_y"))
  {
    mooseDeprecated("StressDivergenceTensors has been updated to accept a string of save_in variable names, e.g. save_in = 'save_in_disp_x save_in_disp_y save_in_disp_z' in the input file.");
    save_in[1] = getParam<std::vector<AuxVariableName> >("save_in_disp_y");
  }
  else if (isParamValid("save_in_disp_z"))
  {
    mooseDeprecated("StressDivergenceTensors has been updated to accept a string of save_in variable names, e.g. save_in = 'save_in_disp_x save_in_disp_y save_in_disp_z' in the input file.");
     save_in[2] = getParam<std::vector<AuxVariableName> >("save_in_disp_z");
  }

  if (isParamValid("save_in") && save_in.size() != _ndisp)
    mooseError("Number of save_in variables should equal to the number of displacement variables: " << _ndisp);

  std::vector<std::vector<AuxVariableName> > diag_save_in(_ndisp);
  if (isParamValid("diag_save_in"))
  {
    std::vector<AuxVariableName> this_diag_save_in = getParam<std::vector<AuxVariableName> >("diag_save_in");
    for (unsigned int i = 0; i < _ndisp; ++i)
      diag_save_in[i].push_back(this_diag_save_in[i]);
  }
  else if (isParamValid("diag_save_in_disp_x"))
  {
    mooseDeprecated("StressDivergenceTensors has been updated to accept a string of diag_save_in variable names, e.g. diag_save_in = 'diag_save_in_disp_x diag_save_in_disp_y diag_save_in_disp_z' in the input file.");
    diag_save_in[0] = getParam<std::vector<AuxVariableName> >("diag_save_in_disp_x");
  }
  else if (isParamValid("diag_save_in_disp_y"))
  {
    mooseDeprecated("StressDivergenceTensors has been updated to accept a string of diag_save_in variable names, e.g. diag_save_in = 'diag_save_in_disp_x diag_save_in_disp_y diag_save_in_disp_z' in the input file.");
    diag_save_in[1] = getParam<std::vector<AuxVariableName> >("diag_save_in_disp_y");
  }
  else if (isParamValid("diag_save_in_disp_z"))
  {
    mooseDeprecated("StressDivergenceTensors has been updated to accept a string of diag_save_in variable names, e.g. diag_save_in = 'diag_save_in_disp_x diag_save_in_disp_y diag_save_in_disp_z' in the input file.");
    diag_save_in[2] = getParam<std::vector<AuxVariableName> >("diag_save_in_disp_z");
  }

  if (isParamValid("diag_save_in") && diag_save_in.size() != _ndisp)
    mooseError("Number of diag_save_in variables should equal to the number of displacement variables: " << _ndisp);

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

  for (unsigned int i = 0; i < _ndisp; ++i)
  {
    std::string kernel_name = "TensorMechanics_" + Moose::stringify(i);

    params.set<unsigned int>("component") = i;
    params.set<NonlinearVariableName>("variable") = displacements[i];
    params.set<std::vector<AuxVariableName> >("save_in") = save_in[i];
    params.set<std::vector<AuxVariableName> >("diag_save_in") = diag_save_in[i];

    addkernel(kernel_name, params);
  }
}

void
TensorMechanicsAction::addkernel(const std::string & name,  InputParameters & params)
{
  _problem->addKernel("StressDivergenceTensors", name, params);
}
