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
#include "MooseMesh.h"

template<>
InputParameters validParams<TensorMechanicsAction>()
{
  InputParameters params = validParams<Action>();
  params.addClassDescription("Set up stress divergence kernels with coordinate system aware logic");
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
    Action(params),
    _displacements(getParam<std::vector<NonlinearVariableName> >("displacements")),
    _ndisp(_displacements.size()),
    _coupled_displacements(_ndisp),
    _save_in(getParam<std::vector<AuxVariableName> >("save_in")),
    _diag_save_in(getParam<std::vector<AuxVariableName> >("diag_save_in"))
{
  // convert vector of NonlinearVariableName to vector of VariableName
  for (unsigned int i = 0; i < _ndisp; ++i)
    _coupled_displacements[i] = _displacements[i];

  if (_save_in.size() != 0 && _save_in.size() != _ndisp)
    mooseError("Number of save_in variables should equal to the number of displacement variables " << _ndisp);

  if (_diag_save_in.size() != 0 && _diag_save_in.size() != _ndisp)
    mooseError("Number of diag_save_in variables should equal to the number of displacement variables " << _ndisp);
}

void
TensorMechanicsAction::act()
{
  // get a list of all subdomains first
  const auto & subdomain_set = _problem->mesh().meshSubdomains();
  std::vector<SubdomainID> subdomains(subdomain_set.begin(), subdomain_set.end());

  // make sure all subdomains are using the same coordinate system
  _coord_system = _problem->getCoordSystem(subdomains[0]);
  for (auto s : subdomains)
    if (_problem->getCoordSystem(s) != _coord_system)
      mooseError("The TensorMechanics action requires all subdomains to have the same coordinate system");

  auto tensor_kernel_type = getKernelType();
  auto params = getParameters(tensor_kernel_type);

  for (unsigned int i = 0; i < _ndisp; ++i)
  {
    std::string kernel_name = "TensorMechanics_" + Moose::stringify(i);

    params.set<unsigned int>("component") = i;
    params.set<NonlinearVariableName>("variable") = _displacements[i];

    if (_save_in.size() != 0)
      params.set<std::vector<AuxVariableName> >("save_in") = {_save_in[i]};
    if (_diag_save_in.size() != 0)
      params.set<std::vector<AuxVariableName> >("diag_save_in") = {_diag_save_in[i]};

    _problem->addKernel(tensor_kernel_type, kernel_name, params);
  }
}

std::string
TensorMechanicsAction::getKernelType()
{
  std::string type;

  // choose kernel type based on coordinate system
  switch (_coord_system)
  {
    case Moose::COORD_XYZ:
      type = "StressDivergenceTensors";
      break;

    case Moose::COORD_RZ:
      type = "StressDivergenceRZTensors";
      break;

    case Moose::COORD_RSPHERICAL:
      type = "StressDivergenceRSphericalTensors";
      break;

    default:
      mooseError("Unsupported coordinate system");
  }

  return type;
}

InputParameters
TensorMechanicsAction::getParameters(std::string type)
{
  InputParameters params = _factory.getValidParams(type);
  params.set<std::vector<VariableName> >("displacements") = _coupled_displacements;

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

  return params;
}
