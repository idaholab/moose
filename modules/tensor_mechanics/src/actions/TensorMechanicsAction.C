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
  params.addParam<NonlinearVariableName>("temp", "The temperature");
  params.addParam<std::string>("base_name", "Material property base name");
  params.addParam<bool>("use_displaced_mesh", false, "Whether to use displaced mesh in the kernels");
  params.addParam<std::vector<SubdomainName> >("block", "The list of ids of the blocks (subdomain) that the stress divergence kernel will be applied to");
  params.addParam<std::vector<AuxVariableName> >("save_in", "Auxiliary variables to save the displacement residuals.");
  params.addParam<std::vector<AuxVariableName> >("diag_save_in", "Auxiliary variables to save the displacement diagonal preconditioner terms.");
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
  else
    mooseError("The input file should specify a string of displacement names; these names should match the Variable block names.");

  unsigned int _ndisp = displacements.size();
  std::vector<VariableName> coupled_displacements;
  for (unsigned int i = 0; i < _ndisp; ++i)
    coupled_displacements.push_back(displacements[i]);

  std::vector<AuxVariableName> this_save_in = getParam<std::vector<AuxVariableName> >("save_in");
  std::vector<std::vector<AuxVariableName> > save_in(_ndisp);
  if (isParamValid("save_in"))
    for (unsigned int i = 0; i < _ndisp; ++i)
      save_in[i].push_back(this_save_in[i]);

  std::vector<AuxVariableName> this_diag_save_in = getParam<std::vector<AuxVariableName> >("diag_save_in");
  std::vector<std::vector<AuxVariableName> > diag_save_in(_ndisp);
  if (isParamValid("diag_save_in"))
    for (unsigned int i = 0; i < _ndisp; ++i)
      diag_save_in[i].push_back(this_diag_save_in[i]);

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
