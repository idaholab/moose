#include "TotalPower.h"

registerMooseObject("THMApp", TotalPower);

template <>
InputParameters
validParams<TotalPower>()
{
  InputParameters params = validParams<TotalPowerBase>();
  params.addRequiredParam<Real>("power", "Number specifying the total power");
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

  _sim.addConstantScalarIC(_power_var_name, _power);
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
    _sim.addAuxScalarKernel(class_name, nm, pars);
    connectObject(pars, nm, "power", "value");
  }
}
