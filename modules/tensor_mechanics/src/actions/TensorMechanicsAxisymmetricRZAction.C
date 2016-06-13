/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "TensorMechanicsAxisymmetricRZAction.h"

#include "Factory.h"
#include "FEProblem.h"
#include "Conversion.h"

template<>
InputParameters validParams<TensorMechanicsAxisymmetricRZAction>()
{
  InputParameters params = validParams<Action>();
  params.addClassDescription("Set up stress divergence kernel for 2D cylindrical problem");
  params.addRequiredParam<std::vector<NonlinearVariableName> >("displacements", "The nonlinear displacement variables for the problem");
  params.addParam<std::string>("base_name", "Material property base name");
  params.addParam<bool>("use_displaced_mesh", false, "Whether to use displaced mesh in the kernels");
  params.addParam<std::vector<SubdomainName> >("block", "The list of ids of the blocks (subdomain) that the stress divergence kernel will be applied to");
  params.addParam<std::vector<AuxVariableName> >("save_in", "The displacement residuals");
  params.addParam<std::vector<AuxVariableName> >("diag_save_in", "The displacement diagonal preconditioner terms");
  return params;
}

TensorMechanicsAxisymmetricRZAction::TensorMechanicsAxisymmetricRZAction(const InputParameters & params) :
    Action(params)
{
}

void
TensorMechanicsAxisymmetricRZAction::act()
{
  std::vector<NonlinearVariableName>displacements = getParam<std::vector<NonlinearVariableName> >("displacements");
  unsigned int _ndisp = displacements.size();

  //Error checking:  Can only take two displacement variables in StressDivergenceRZTensors kernel
  if (_ndisp != 2)
    mooseError("Number of displacement variables should be 2 but recieved " << _ndisp);

  std::vector<VariableName>coupled_displacements;
  for (unsigned int i = 0; i < _ndisp; ++i)
    coupled_displacements.push_back(displacements[i]);

  std::vector<AuxVariableName>save_in = getParam<std::vector<AuxVariableName> >("save_in");
  if (isParamValid("save_in") && save_in.size() != _ndisp)
    mooseError("Number of save_in variables should equal to the number of displacement variables " << _ndisp);

  std::vector<AuxVariableName>diag_save_in = getParam<std::vector<AuxVariableName> >("diag_save_in");
  if (isParamValid("diag_save_in") && diag_save_in.size() != _ndisp)
    mooseError("Number of diag_save_in variables should equal to the number of displacement variables " << _ndisp);

  InputParameters params = _factory.getValidParams("StressDivergenceRZTensors");
  params.set<std::vector<VariableName> >("displacements") = coupled_displacements;

  params.set<bool>("use_displaced_mesh") = getParam<bool>("use_displaced_mesh");

  if (isParamValid("base_name"))
    params.set<std::string>("base_name") = getParam<std::string>("base_name");

// Check whether this StressDivergenceRZTensors kernel is restricted to certain block?
  if (isParamValid("block"))
    params.set<std::vector<SubdomainName> >("block") = getParam<std::vector<SubdomainName> >("block");

  for (unsigned int i = 0; i < _ndisp; ++i)
  {
    std::string kernel_name = "TensorMechanicsAxisymmetricRZ_" + Moose::stringify(i);

    params.set<unsigned int>("component") = i;
    params.set<NonlinearVariableName>("variable") = displacements[i];
  if (isParamValid("save_in"))
    params.set<std::vector<AuxVariableName> >("save_in") = {save_in[i]};
  if (isParamValid("diag_save_in"))
    params.set<std::vector<AuxVariableName> >("diag_save_in") = {diag_save_in[i]};

    _problem->addKernel("StressDivergenceRZTensors", kernel_name, params);
  }
}
