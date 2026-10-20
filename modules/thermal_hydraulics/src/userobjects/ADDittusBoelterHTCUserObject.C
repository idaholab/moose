//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADDittusBoelterHTCUserObject.h"

#include "ScalarCoupleable.h"
#include "SinglePhaseFluidProperties.h"

registerMooseObject("ThermalHydraulicsApp", ADDittusBoelterHTCUserObject);

InputParameters
ADDittusBoelterHTCUserObject::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params += ADFunctorInterface::validParams();
  params.addClassDescription(
      "Computes the wall heat transfer coefficient of a path-integrated flow segment using "
      "the Dittus-Boelter correlation, and supplies it as a functor named after this object.");
  params.addCoupledVar(
      "mass_flow_rate", {}, "Mass flow rate through the segment. Takes a scalar variable name");
  params.addCoupledVar(
      "T_fluid", {}, "Fluid temperature of the segment. Takes a scalar variable name");
  params.addCoupledVar(
      "T_fluid_inlet",
      {},
      "Fluid temperature of the nominal inlet segment/component. Takes a scalar variable name");
  params.addCoupledVar(
      "T_fluid_outlet",
      {},
      "Fluid temperature of the nominal outlet segment/component. Takes a scalar variable name");
  params.addRequiredParam<UserObjectName>("fluid_properties",
                                          "The name of the user object for fluid properties");
  params.addRequiredParam<MooseFunctorName>("reference_pressure", "System reference pressure [Pa]");
  params.addRequiredParam<MooseFunctorName>("flow_area",
                                            "Cross-sectional area of the fluid flow channel [m^2]");
  params.addRequiredParam<MooseFunctorName>("wetted_perimeter",
                                            "Wetted perimeter of the fluid flow channel [m]");

  return params;
}

ADDittusBoelterHTCUserObject::ADDittusBoelterHTCUserObject(const InputParameters & parameters)
  : GeneralUserObject(parameters),
    ADFunctorInterface(this),
    _htc(name(),
         getUserObject<SinglePhaseFluidProperties>("fluid_properties"),
         ScalarCoupleable::adCoupledScalarValue("mass_flow_rate"),
         ScalarCoupleable::adCoupledScalarValue("T_fluid"),
         ScalarCoupleable::adCoupledScalarValue("T_fluid_inlet"),
         ScalarCoupleable::adCoupledScalarValue("T_fluid_outlet"),
         getFunctor<ADReal>("reference_pressure"),
         getFunctor<ADReal>("flow_area"),
         getFunctor<ADReal>("wetted_perimeter"))
{
  // Registered here rather than in initialSetup() because SubProblem::initialSetup() checks
  // that every requested functor has been supplied, and runs before user objects are set up
  for (const auto tid : make_range(_subproblem.numThreads()))
    _subproblem.addFunctor(name(), _htc, tid);
}
