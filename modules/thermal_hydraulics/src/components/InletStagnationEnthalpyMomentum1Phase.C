//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "InletStagnationEnthalpyMomentum1Phase.h"
#include "FlowModelSinglePhase.h"

registerMooseObject("ThermalHydraulicsApp", InletStagnationEnthalpyMomentum1Phase);

InputParameters
InletStagnationEnthalpyMomentum1Phase::validParams()
{
  InputParameters params = FlowBoundary1Phase::validParams();
  params.addRequiredParam<Real>("rhou", "Prescribed momentum density [kg/(m^2-s)]");
  params.addRequiredParam<Real>("H", "Prescribed specific total enthalpy [J/kg]");
  params.addParam<bool>("reversible", false, "True for reversible, false (default) for pure inlet");
  params.addClassDescription("Boundary condition with prescribed stagnation enthalpy and momentum "
                             "for 1-phase flow channels.");
  return params;
}

InletStagnationEnthalpyMomentum1Phase::InletStagnationEnthalpyMomentum1Phase(
    const InputParameters & params)
  : FlowBoundary1Phase(params), _reversible(getParam<bool>("reversible"))
{
}

void
InletStagnationEnthalpyMomentum1Phase::check() const
{
  FlowBoundary1Phase::check();

  logError("This component does not work with rDG, yet.");
}

void
InletStagnationEnthalpyMomentum1Phase::addMooseObjects()
{
}
