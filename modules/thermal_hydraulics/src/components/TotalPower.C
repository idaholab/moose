//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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
  params.addRequiredParam<FunctionName>("power", "Total power [W]");
  params.addClassDescription("Prescribes total power via a user supplied function");
  return params;
}

TotalPower::TotalPower(const InputParameters & parameters)
  : TotalPowerBase(parameters), _power_fn_name(getParam<FunctionName>("power"))
{
}

void
TotalPower::addVariables()
{
  TotalPowerBase::addVariables();

  if (!_app.isRestarting())
    getTHMProblem().addFunctionScalarIC(_power_var_name, _power_fn_name);
}

void
TotalPower::addMooseObjects()
{
  {
    std::string class_name = "FunctionScalarAux";
    InputParameters pars = _factory.getValidParams(class_name);
    pars.set<AuxVariableName>("variable") = _power_var_name;
    pars.set<std::vector<FunctionName>>("function") = {_power_fn_name};
    std::string nm = genName(name(), "power_aux");
    getTHMProblem().addAuxScalarKernel(class_name, nm, pars);
  }
}
