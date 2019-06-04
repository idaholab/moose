#include "PrescribedReactorPower.h"
#include "Function.h"

registerMooseObject("THMApp", PrescribedReactorPower);

template <>
InputParameters
validParams<PrescribedReactorPower>()
{
  InputParameters params = validParams<ReactorPower>();
  params.addRequiredParam<Real>("power", "Number specifying the total reactor power");
  return params;
}

PrescribedReactorPower::PrescribedReactorPower(const InputParameters & parameters)
  : ReactorPower(parameters), _power(getParam<Real>("power"))
{
}

void
PrescribedReactorPower::addVariables()
{
  ReactorPower::addVariables();

  _sim.addConstantScalarIC(_power_var_name, _power);
}

void
PrescribedReactorPower::addMooseObjects()
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
