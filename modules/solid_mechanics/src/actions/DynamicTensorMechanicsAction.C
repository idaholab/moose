//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DynamicTensorMechanicsAction.h"
#include "Factory.h"
#include "FEProblem.h"
#include "Parser.h"

registerMooseAction("TensorMechanicsApp", DynamicTensorMechanicsAction, "meta_action");

registerMooseAction("TensorMechanicsApp", DynamicTensorMechanicsAction, "setup_mesh_complete");

registerMooseAction("TensorMechanicsApp",
                    DynamicTensorMechanicsAction,
                    "validate_coordinate_systems");

registerMooseAction("TensorMechanicsApp", DynamicTensorMechanicsAction, "add_variable");

registerMooseAction("TensorMechanicsApp", DynamicTensorMechanicsAction, "add_aux_variable");

registerMooseAction("TensorMechanicsApp", DynamicTensorMechanicsAction, "add_kernel");

registerMooseAction("TensorMechanicsApp", DynamicTensorMechanicsAction, "add_aux_kernel");

registerMooseAction("TensorMechanicsApp", DynamicTensorMechanicsAction, "add_material");

registerMooseAction("TensorMechanicsApp",
                    DynamicTensorMechanicsAction,
                    "add_master_action_material");

InputParameters
DynamicTensorMechanicsAction::validParams()
{
  InputParameters params = TensorMechanicsAction::validParams();
  params.addClassDescription("Set up dynamic stress divergence kernels");
  params.addParam<bool>("static_initialization",
                        false,
                        "Set to true get the system to "
                        "equilibrium under gravity by running a "
                        "quasi-static analysis (by solving Ku = F) "
                        "in the first time step.");

  params.addParam<std::vector<AuxVariableName>>(
      "velocities",
      std::vector<AuxVariableName>({"vel_x", "vel_y", "vel_z"}),
      "Names of the velocity variables");
  params.addParam<std::vector<AuxVariableName>>(
      "accelerations",
      std::vector<AuxVariableName>({"accel_x", "accel_y", "accel_z"}),
      "Names of the acceleration variables");

  params.addParam<Real>("hht_alpha",
                        0,
                        "alpha parameter for mass dependent numerical damping induced "
                        "by HHT time integration scheme");
  params.addParam<Real>("newmark_beta", 0.25, "beta parameter for Newmark Time integration");
  params.addParam<Real>("newmark_gamma", 0.5, "gamma parameter for Newmark Time integration");
  params.addParam<MaterialPropertyName>("mass_damping_coefficient",
                                        0.0,
                                        "Name of material property or a constant real "
                                        "number defining mass Rayleigh parameter (eta).");
  params.addParam<MaterialPropertyName>("stiffness_damping_coefficient",
                                        0.0,
                                        "Name of material property or a constant real "
                                        "number defining stiffness Rayleigh parameter (zeta).");
  params.addParam<MaterialPropertyName>(
      "density", "density", "Name of Material Property that provides the density");
  params.addParamNamesToGroup("hht_alpha newmark_beta newmark_gamma",
                              "Time integration parameters");
  // Deprecated parameters
  params.addDeprecatedParam<Real>("alpha",
                                  "alpha parameter for mass dependent numerical damping induced "
                                  "by HHT time integration scheme",
                                  "Please use hht_alpha");
  params.addDeprecatedParam<Real>(
      "beta", "beta parameter for Newmark Time integration", "Please use newmark_beta");
  params.addDeprecatedParam<Real>(
      "gamma", "gamma parameter for Newmark Time integration", "Please use newmark_gamma");
  params.addDeprecatedParam<MaterialPropertyName>("eta",
                                                  "Name of material property or a constant real "
                                                  "number defining mass Rayleigh parameter (eta).",
                                                  "Please use mass_damping_coefficient");
  params.addDeprecatedParam<MaterialPropertyName>(
      "zeta",
      "Name of material property or a constant real "
      "number defining stiffness Rayleigh parameter (zeta).",
      "Please use stiffness_damping_coefficient");

  return params;
}

DynamicTensorMechanicsAction::DynamicTensorMechanicsAction(const InputParameters & params)
  : TensorMechanicsAction(params),
    _velocities(getParam<std::vector<AuxVariableName>>("velocities")),
    _accelerations(getParam<std::vector<AuxVariableName>>("accelerations")),
    _newmark_beta(isParamValid("beta") ? getParam<Real>("beta") : getParam<Real>("newmark_beta")),
    _newmark_gamma(isParamValid("gamma") ? getParam<Real>("gamma")
                                         : getParam<Real>("newmark_gamma")),
    _hht_alpha(isParamValid("alpha") ? getParam<Real>("alpha") : getParam<Real>("hht_alpha"))

{
}

void
DynamicTensorMechanicsAction::act()
{
  const std::array<std::string, 3> dir{{"x", "y", "z"}};

  if (_velocities.size() < _ndisp)
    paramError("velocities", "Supply one velocity variable per displacement direction");
  if (_accelerations.size() < _ndisp)
    paramError("accelerations", "Supply one acceleration variable per displacement direction");

  // Add aux variables for velocities and accelerations
  if (_current_task == "add_aux_variable" && getParam<bool>("add_variables"))
  {
    auto params = _factory.getValidParams("MooseVariable");
    // determine necessary order
    const bool second = _problem->mesh().hasSecondOrderElements();

    params.set<MooseEnum>("order") = second ? "SECOND" : "FIRST";
    params.set<MooseEnum>("family") = "LAGRANGE";

    for (unsigned int i = 0; i < _ndisp; ++i)
    {
      _problem->addAuxVariable("MooseVariable", _velocities[i], params);
      _problem->addAuxVariable("MooseVariable", _accelerations[i], params);
    }
  }

  // Add aux kernel for velocities and accelerations
  if (_current_task == "add_aux_kernel")
  {
    //
    // Note: AuxKernels that are limited to TIMESTEP_END to not get their dependencies
    // resolved automatically. Thus we _must_ construct the acceleration kernels _first_.
    // NewmarkAccelAux only uses the old velocity.
    //

    // acceleration aux kernels
    for (unsigned int i = 0; i < _ndisp; ++i)
    {
      auto kernel_type = "NewmarkAccelAux";
      auto params = _factory.getValidParams(kernel_type);
      params.set<AuxVariableName>("variable") = _accelerations[i];
      params.set<std::vector<VariableName>>("displacement") = {_displacements[i]};
      params.set<std::vector<VariableName>>("velocity") = {_velocities[i]};
      params.set<ExecFlagEnum>("execute_on") = EXEC_TIMESTEP_END;
      params.set<Real>("beta") = _newmark_beta;
      params.applyParameters(parameters());
      _problem->addAuxKernel(kernel_type, "TM_" + name() + '_' + _accelerations[i], params);
    }

    // velocity aux kernels
    for (unsigned int i = 0; i < _ndisp; ++i)
    {
      auto kernel_type = "NewmarkVelAux";
      auto params = _factory.getValidParams(kernel_type);
      params.set<AuxVariableName>("variable") = _velocities[i];
      params.set<std::vector<VariableName>>("acceleration") = {_accelerations[i]};
      params.set<ExecFlagEnum>("execute_on") = EXEC_TIMESTEP_END;
      params.set<Real>("gamma") = _newmark_gamma;
      params.applyParameters(parameters());
      _problem->addAuxKernel(kernel_type, "TM_" + name() + '_' + _velocities[i], params);
    }
  }

  // add inertia kernel
  if (_current_task == "add_kernel")
  {
    for (unsigned int i = 0; i < _ndisp; ++i)
    {
      auto kernel_type = _use_ad ? "ADInertialForce" : "InertialForce";
      auto params = _factory.getValidParams(kernel_type);

      params.set<NonlinearVariableName>("variable") = _displacements[i];
      params.set<std::vector<VariableName>>("velocity") = {_velocities[i]};
      params.set<std::vector<VariableName>>("acceleration") = {_accelerations[i]};
      params.set<bool>("use_displaced_mesh") = false;
      params.set<Real>("beta") = _newmark_beta;
      params.set<Real>("gamma") = _newmark_gamma;
      params.set<Real>("alpha") = _hht_alpha;
      params.set<MaterialPropertyName>("eta") =
          isParamValid("eta") ? getParam<MaterialPropertyName>("eta")
                              : getParam<MaterialPropertyName>("mass_damping_coefficient");

      params.applyParameters(parameters());

      _problem->addKernel(kernel_type, "TM_" + name() + "_inertia_" + dir[i], params);
    }
  }

  // call parent class method
  TensorMechanicsAction::act();
}

std::string
DynamicTensorMechanicsAction::getKernelType()
{
  // choose kernel type based on coordinate system
  if (_coord_system == Moose::COORD_XYZ)
    return "DynamicStressDivergenceTensors";
  else
    mooseError("Unsupported coordinate system");
}

InputParameters
DynamicTensorMechanicsAction::getKernelParameters(std::string type)
{
  TensorMechanicsAction::getKernelParameters(type);
  InputParameters params = _factory.getValidParams(type);
  params.applyParameters(parameters(), {"zeta", "alpha"});

  params.set<Real>("alpha") =
      isParamValid("alpha") ? getParam<Real>("alpha") : getParam<Real>("hht_alpha");
  params.set<MaterialPropertyName>("zeta") =
      isParamValid("zeta") ? getParam<MaterialPropertyName>("zeta")
                           : getParam<MaterialPropertyName>("stiffness_damping_coefficient");

  return params;
}
