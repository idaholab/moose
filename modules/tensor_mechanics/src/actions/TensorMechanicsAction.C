/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "TensorMechanicsAction.h"

#include "Factory.h"
#include "FEProblem.h"
#include "Conversion.h"

template<>
InputParameters validParams<TensorMechanicsAction>()
{
  InputParameters params = validParams<Action>();
  params.addClassDescription("Set up stress divergence kernels");
  params.addRequiredParam<std::vector<NonlinearVariableName> >("displacements", "The nonlinear displacement variables for the problem");
  params.addParam<NonlinearVariableName>("temp", "The temperature");
  params.addParam<std::string>("base_name", "Material property base name");
  params.addParam<bool>("use_finite_deform_jacobian", false, "Jacobian for corrotational finite strain");
  params.addParam<bool>("use_displaced_mesh", false, "Whether to use displaced mesh in the kernels");
  params.addParam<std::vector<SubdomainName> >("block", "The list of ids of the blocks (subdomain) that the stress divergence kernel will be applied to");
  params.addParam<std::vector<AuxVariableName> >("save_in", "The displacement residuals");
  params.addParam<std::vector<AuxVariableName> >("diag_save_in", "The displacement diagonal preconditioner terms");
  return params;
}

TensorMechanicsAction::TensorMechanicsAction(const InputParameters & params) :
    Action(params)
{
}

void
TensorMechanicsAction::act()
{
  std::vector<NonlinearVariableName>displacements = getParam<std::vector<NonlinearVariableName> >("displacements");
  unsigned int _ndisp = displacements.size();

  std::vector<VariableName>coupled_displacements;
  for (unsigned int i = 0; i < _ndisp; ++i)
    coupled_displacements.push_back(displacements[i]);

  std::vector<AuxVariableName>save_in = getParam<std::vector<AuxVariableName> >("save_in");
  if (isParamValid("save_in") && save_in.size() != _ndisp)
    mooseError("Number of save_in variables should equal to the number of displacement variables " << _ndisp);

  std::vector<AuxVariableName>diag_save_in = getParam<std::vector<AuxVariableName> >("diag_save_in");
  if (isParamValid("diag_save_in") && diag_save_in.size() != _ndisp)
    mooseError("Number of diag_save_in variables should equal to the number of displacement variables " << _ndisp);

  InputParameters params = _factory.getValidParams("StressDivergenceTensors");
  params.set<std::vector<VariableName> >("displacements") = coupled_displacements;

  if (isParamValid("temp"))
    params.addCoupledVar("temp", "");

  params.set<bool>("use_displaced_mesh") = getParam<bool>("use_displaced_mesh");

  if (isParamValid("base_name"))
    params.set<std::string>("base_name") = getParam<std::string>("base_name");

  if (isParamValid("use_finite_deform_jacobian"))
    params.set<bool>("use_finite_deform_jacobian") = getParam<bool>("use_finite_deform_jacobian");

// Check whether this StressDivergenceTensor kernel is restricted to certain block?
  if (isParamValid("block"))
    params.set<std::vector<SubdomainName> >("block") = getParam<std::vector<SubdomainName> >("block");

  for (unsigned int i = 0; i < _ndisp; ++i)
  {
    std::string kernel_name = "TensorMechanics_" + Moose::stringify(i);

    params.set<unsigned int>("component") = i;
    params.set<NonlinearVariableName>("variable") = displacements[i];
    if (isParamValid("save_in"))
      params.set<std::vector<AuxVariableName> >("save_in") = {save_in[i]};
    if (isParamValid("diag_save_in"))
      params.set<std::vector<AuxVariableName> >("diag_save_in") = {diag_save_in[i]};

    addkernel(kernel_name, params);
  }
}

void
TensorMechanicsAction::addkernel(const std::string & name,  InputParameters & params)
{
  _problem->addKernel("StressDivergenceTensors", name, params);
}
