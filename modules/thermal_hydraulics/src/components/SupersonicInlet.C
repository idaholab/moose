//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SupersonicInlet.h"
#include "FlowModelSinglePhase.h"
#include "SinglePhaseFluidProperties.h"

registerMooseObject("ThermalHydraulicsApp", SupersonicInlet);

InputParameters
SupersonicInlet::validParams()
{
  InputParameters params = FlowBoundary1Phase::validParams();
  params.addParam<Real>("p", "Prescribed pressure [Pa]");
  params.addParam<Real>("T", "Prescribed temperature [K]");
  params.addParam<Real>("vel", "Prescribed velocity [m/s]");
  return params;
}

SupersonicInlet::SupersonicInlet(const InputParameters & parameters)
  : FlowBoundary1Phase(parameters)
{
}

void
SupersonicInlet::check() const
{
  FlowBoundary1Phase::check();

  logError("This component is deprecated.");
}

void
SupersonicInlet::addMooseObjects()
{
}
