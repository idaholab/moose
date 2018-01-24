/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "DynamicTensorMechanicsAction.h"
#include "Factory.h"
#include "FEProblem.h"
#include "Parser.h"

template <>
InputParameters
validParams<DynamicTensorMechanicsAction>()
{
  InputParameters params = validParams<TensorMechanicsAction>();
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
                        "equillibrium under gravity by running a "
                        "quasi-static analysis (by solving Ku = F) "
                        "in the first time step.");
  return params;
}

DynamicTensorMechanicsAction::DynamicTensorMechanicsAction(const InputParameters & params)
  : TensorMechanicsAction(params)
{
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
