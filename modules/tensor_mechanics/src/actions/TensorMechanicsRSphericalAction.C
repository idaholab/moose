/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "TensorMechanicsRSphericalAction.h"

#include "Factory.h"
#include "FEProblem.h"
#include "Parser.h"
#include "Conversion.h"

template<>
InputParameters validParams<TensorMechanicsRSphericalAction>()
{
  InputParameters params = validParams<Action>();
  params.addClassDescription("Set up stress divergence kernel for the 1D spherical problem");
  params.addRequiredParam<std::vector<NonlinearVariableName> >("displacements", "The nonlinear displacement variable for the problem (should only be a single variable)");
  params.addParam<std::string>("base_name", "Material property base name");
  params.addParam<bool>("use_displaced_mesh", false, "Whether to use displaced mesh in the kernels");
  params.addParam<std::vector<SubdomainName> >("block", "The list of ids of the blocks (subdomain) that the stress divergence kernel will be applied to");
  params.addParam<std::vector<AuxVariableName> >("save_in", "Auxiliary variables to save the r displacement residuals.");
  params.addParam<std::vector<AuxVariableName> >("diag_save_in", "Auxiliary variables to save the r displacement diagonal preconditoner terms.");
  return params;
}

TensorMechanicsRSphericalAction::TensorMechanicsRSphericalAction(const InputParameters & params) :
    Action(params)
{
}

void
TensorMechanicsRSphericalAction::act()
{
  std::vector<NonlinearVariableName> displacements = getParam<std::vector<NonlinearVariableName> > ("displacements");
  unsigned int _ndisp = displacements.size();

  // Error checking:  Can only take one displacement variable in StressDivergenceRSphericalTensors kernel
  if (_ndisp != 1)
    mooseError("Number of displacement variable should be 1 but recieved " << _ndisp);

  std::vector<VariableName> coupled_displacements;
  for (unsigned int i = 0; i < _ndisp; ++i)
    coupled_displacements.push_back(displacements[i]);

  std::vector<AuxVariableName> this_save_in = getParam<std::vector<AuxVariableName> >("save_in");
  if (isParamValid("save_in") && this_save_in.size() != _ndisp)
    mooseError("Number of save_in variables should equal to the number of displacement variables: " << _ndisp);
  std::vector<std::vector<AuxVariableName> > save_in(_ndisp);
  if (isParamValid("save_in"))
    for (unsigned int i = 0; i < _ndisp; ++i)
      save_in[i].push_back(this_save_in[i]);

  std::vector<AuxVariableName> this_diag_save_in = getParam<std::vector<AuxVariableName> >("diag_save_in");
  if (isParamValid("diag_save_in") && this_diag_save_in.size() != _ndisp)
    mooseError("Number of diag_save_in variables should equal to the number of displacement variables: " << _ndisp);
  std::vector<std::vector<AuxVariableName> > diag_save_in(_ndisp);
  if (isParamValid("diag_save_in"))
    for (unsigned int i = 0; i < _ndisp; ++i)
      diag_save_in[i].push_back(this_diag_save_in[i]);

  InputParameters params = _factory.getValidParams("StressDivergenceRSphericalTensors");
  params.set<std::vector<VariableName> >("displacements") = coupled_displacements;

  params.set<bool>("use_displaced_mesh") = getParam<bool>("use_displaced_mesh");

  if (isParamValid("base_name"))
    params.set<std::string>("base_name") = getParam<std::string>("base_name");

// Check whether this StressDivergenceRSphericalTensors kernel is restricted to certain block?
  if (isParamValid("block"))
    params.set<std::vector<SubdomainName> >("block") = getParam<std::vector<SubdomainName> >("block");

  for (unsigned int i = 0; i < _ndisp; ++i)
  {
    std::string kernel_name = "TensorMechanicsRSpherical_" + Moose::stringify(i);

    params.set<unsigned int>("component") = i;
    params.set<NonlinearVariableName>("variable") = displacements[i];
    params.set<std::vector<AuxVariableName> >("save_in") = save_in[i];
    params.set<std::vector<AuxVariableName> >("diag_save_in") = diag_save_in[i];

    _problem->addKernel("StressDivergenceRSphericalTensors", kernel_name, params);
  }
}
