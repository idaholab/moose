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
  params.addParam<bool>(
      "use_displaced_mesh", false, "Whether to use displaced mesh in the kernels");
  return params;
}

DynamicTensorMechanicsAction::DynamicTensorMechanicsAction(const InputParameters & params)
  : TensorMechanicsAction(params)
{
}

void
DynamicTensorMechanicsAction::act()
{
  // Add aux variables for acceleration
  if (_current_task == "add_aux_variable")
  {
  }

  // Add aux kernel for acceleration
  if (_current_task == "add_aux_variable")
  {
  }

  // add inertia kernel
  if (_current_task == "add_kernel")
  {
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
