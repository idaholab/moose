//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TotalPower.h"

registerMooseObject("ThermalHydraulicsApp", TotalPower);

InputParameters
TotalPower::validParams()
{
  InputParameters params = TotalPowerBase::validParams();
  params.addRequiredParam<Real>("power", "Total power [W]");
  params.declareControllable("power");
  params.addClassDescription("Prescribes total power via a user supplied value");
  return params;
}

TotalPower::TotalPower(const InputParameters & parameters)
  : TotalPowerBase(parameters), _power(getParam<Real>("power"))
{
}

void
TotalPower::addVariables()
{
  TotalPowerBase::addVariables();

  getTHMProblem().addConstantScalarIC(_power_var_name, _power);
}

void
TotalPower::addMooseObjects()
{
  {
    std::string class_name = "ConstantScalarAux";
    InputParameters pars = _factory.getValidParams(class_name);
    pars.set<AuxVariableName>("variable") = _power_var_name;
    pars.set<Real>("value") = _power;
    std::string nm = genName(name(), "power_aux");
    getTHMProblem().addAuxScalarKernel(class_name, nm, pars);
    connectObject(pars, nm, "power", "value");
  }
}
