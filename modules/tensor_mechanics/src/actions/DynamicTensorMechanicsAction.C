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

void
DynamicTensorMechanicsAction::addkernel(const std::string & name, InputParameters & params)
{
  //Add the zeta and alpha parameters to the params (which belongs to StressDivergenceTensors).
  //Add DynamicStressDivergenceTensors kernel
  params.addParam<Real>("zeta", 0, "zeta parameter for the Rayleigh damping");
  params.addParam<Real>("alpha", 0, "alpha parameter for HHT time integration");

  params.set<Real>("zeta") = getParam<Real>("zeta");
  params.set<Real>("alpha") = getParam<Real>("alpha");

  _problem->addKernel("DynamicStressDivergenceTensors", name, params);
}

void
DynamicTensorMechanicsAction::act()
{
  TensorMechanicsAction::act();
}

