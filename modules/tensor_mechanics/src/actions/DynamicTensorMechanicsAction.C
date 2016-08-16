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

template<>
InputParameters validParams<DynamicTensorMechanicsAction>()
{
  InputParameters params = validParams<TensorMechanicsAction>();
  params.addClassDescription("Set up dynamic stress divergence kernels");
  params.addParam<Real>("zeta", 0, "zeta parameter for the Rayleigh damping");
  params.addParam<Real>("alpha", 0, "alpha parameter for HHT time integration");
  return params;
}

DynamicTensorMechanicsAction::DynamicTensorMechanicsAction(const InputParameters & params) :
  TensorMechanicsAction(params)
{
}

std::string
DynamicTensorMechanicsAction::getKernelType()
{
  std::string type;

  // choose kernel type based on coordinate system
  switch (_coord_system)
  {
    case Moose::COORD_XYZ:
      type = "DynamicStressDivergenceTensors";
      break;

    default:
      mooseError("Unsupported coordinate system");
  }

  return type;
}

InputParameters
DynamicTensorMechanicsAction::getParameters(std::string type)
{
  InputParameters params = TensorMechanicsAction::getParameters(type);

  params.addParam<Real>("zeta", 0, "zeta parameter for the Rayleigh damping");
  params.addParam<Real>("alpha", 0, "alpha parameter for HHT time integration");

  params.set<Real>("zeta") = getParam<Real>("zeta");
  params.set<Real>("alpha") = getParam<Real>("alpha");

  return params;
}
