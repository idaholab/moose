//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DynamicSolidMechanicsPhysics.h"
#include "Factory.h"
#include "FEProblem.h"
#include "Parser.h"

registerMooseAction("SolidMechanicsApp", DynamicSolidMechanicsPhysics, "meta_action");

registerMooseAction("SolidMechanicsApp", DynamicSolidMechanicsPhysics, "setup_mesh_complete");

registerMooseAction("SolidMechanicsApp",
                    DynamicSolidMechanicsPhysics,
                    "validate_coordinate_systems");

registerMooseAction("SolidMechanicsApp", DynamicSolidMechanicsPhysics, "add_variable");

registerMooseAction("SolidMechanicsApp", DynamicSolidMechanicsPhysics, "add_aux_variable");

registerMooseAction("SolidMechanicsApp", DynamicSolidMechanicsPhysics, "add_kernel");

registerMooseAction("SolidMechanicsApp", DynamicSolidMechanicsPhysics, "add_aux_kernel");

registerMooseAction("SolidMechanicsApp", DynamicSolidMechanicsPhysics, "add_material");

registerMooseAction("SolidMechanicsApp",
                    DynamicSolidMechanicsPhysics,
                    "add_master_action_material");

InputParameters
DynamicSolidMechanicsPhysics::validParams()
{
  InputParameters params = QuasiStaticSolidMechanicsPhysics::validParams();
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
  params.addParamNamesToGroup("velocities accelerations", "Variables");

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

  return params;
}

DynamicSolidMechanicsPhysics::DynamicSolidMechanicsPhysics(const InputParameters & params)
  : QuasiStaticSolidMechanicsPhysics(params),
    _velocities(getParam<std::vector<AuxVariableName>>("velocities")),
    _accelerations(getParam<std::vector<AuxVariableName>>("accelerations"))
{
}

void
DynamicSolidMechanicsPhysics::act()
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
      params.set<Real>("beta") = getParam<Real>("newmark_beta");
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
      params.set<Real>("gamma") = getParam<Real>("newmark_gamma");
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
      params.set<Real>("beta") = getParam<Real>("newmark_beta");
      params.set<Real>("gamma") = getParam<Real>("newmark_gamma");
      params.set<Real>("alpha") = getParam<Real>("hht_alpha");
      params.set<MaterialPropertyName>("eta") =
          getParam<MaterialPropertyName>("mass_damping_coefficient");

      params.applyParameters(parameters());

      _problem->addKernel(kernel_type, "TM_" + name() + "_inertia_" + dir[i], params);
    }
  }

  // call parent class method
  QuasiStaticSolidMechanicsPhysics::act();
}

std::string
DynamicSolidMechanicsPhysics::getKernelType()
{
  // choose kernel type based on coordinate system
  if (_coord_system == Moose::COORD_XYZ)
    return "DynamicStressDivergenceTensors";
  else
    mooseError("Unsupported coordinate system");
}

InputParameters
DynamicSolidMechanicsPhysics::getKernelParameters(std::string type)
{
  InputParameters params = QuasiStaticSolidMechanicsPhysics::getKernelParameters(type);

  if (params.isParamDefined("alpha"))
    params.set<Real>("alpha") = getParam<Real>("hht_alpha");
  if (params.isParamDefined("zeta"))
    params.set<MaterialPropertyName>("zeta") =
        getParam<MaterialPropertyName>("stiffness_damping_coefficient");

  return params;
}
