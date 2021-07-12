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
  params.addParam<MaterialPropertyName>("zeta",
                                        0.0,
                                        "Name of material property or a constant real "
                                        "number defining the zeta parameter for the "
                                        "Rayleigh damping.");
  params.addParam<Real>("alpha", 0, "alpha parameter for HHT time integration");
  params.addParam<bool>("static_initialization",
                        false,
                        "Set to true get the system to "
                        "equilibrium under gravity by running a "
                        "quasi-static analysis (by solving Ku = F) "
                        "in the first time step.");

  params.addParam<std::vector<AuxVariableName>>("velocities", "");
  params.addParam<std::vector<AuxVariableName>>("accelerations", "");

  params.addParam<Real>("beta", 0.25, "beta parameter for Newmark Time integration");
  params.addParam<Real>("gamma", 0.5, "gamma parameter for Newmark Time integration");
  params.addParam<MaterialPropertyName>("eta",
                                        0.0,
                                        "Name of material property or a constant real "
                                        "number defining the eta parameter for the "
                                        "Rayleigh damping.");

  params.addParam<MaterialPropertyName>(
      "density", "density", "Name of Material Property that provides the density");
  return params;
}

DynamicTensorMechanicsAction::DynamicTensorMechanicsAction(const InputParameters & params)
  : TensorMechanicsAction(params),
    _velocities(getParam<std::vector<AuxVariableName>>("velocities")),
    _accelerations(getParam<std::vector<AuxVariableName>>("accelerations"))
{
}

void
DynamicTensorMechanicsAction::act()
{
  const std::array<std::string, 3> dir{"x", "y", "z"};

  // error check vecolities and accelerations parameters
  if (_velocities.size() != _ndisp)
    paramError("velocities", "Supply one velocity variable per displacement direction");
  if (_accelerations.size() != _ndisp)
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
      _problem->addVariable("MooseVariable", _velocities[i], params);
      _problem->addVariable("MooseVariable", _accelerations[i], params);
    }
  }

  // Add aux kernel for velocities and accelerations
  if (_current_task == "add_aux_kernel")
  {
    // velocity aux kernels
    for (unsigned int i = 0; i < _ndisp; ++i)
    {
      auto kernel_type = "NewmarkVelAux";
      auto params = getKernelParameters(kernel_type);
      params.set<NonlinearVariableName>("variable") = _velocities[i];
      params.set<std::vector<VariableName>>("acceleration") = {_accelerations[i]};
      params.set<ExecFlagEnum>("execute_on") = EXEC_TIMESTEP_END;
      params.applyParameters(parameters());
      _problem->addAuxKernel(kernel_type, "TM_" + name() + '_' + _velocities[i], params);
    }

    // acceleration aux kernels
    for (unsigned int i = 0; i < _ndisp; ++i)
    {
      auto kernel_type = "NewmarkAccelAux";
      auto params = getKernelParameters(kernel_type);
      params.set<NonlinearVariableName>("variable") = _accelerations[i];
      params.set<std::vector<VariableName>>("displacement") = {_displacements[i]};
      params.set<std::vector<VariableName>>("velocity") = {_velocities[i]};
      params.set<ExecFlagEnum>("execute_on") = EXEC_TIMESTEP_END;
      params.applyParameters(parameters());
      _problem->addAuxKernel(kernel_type, "TM_" + name() + '_' + _accelerations[i], params);
    }
  }

  // add inertia kernel
  if (_current_task == "add_kernel")
  {
    for (unsigned int i = 0; i < _ndisp; ++i)
    {
      auto kernel_type = "InertialForce";
      auto params = getKernelParameters(kernel_type);

      params.set<NonlinearVariableName>("variable") = _displacements[i];
      params.set<std::vector<VariableName>>("velocity") = {_velocities[i]};
      params.set<std::vector<VariableName>>("acceleration") = {_accelerations[i]};
      params.applyParameters(parameters());

      _problem->addKernel(kernel_type, "TM_" + name() + '_' + dir[i], params);
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
