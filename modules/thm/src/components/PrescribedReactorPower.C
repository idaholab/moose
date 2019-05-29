#include "PrescribedReactorPower.h"
#include "Function.h"

registerMooseObject("THMApp", PrescribedReactorPower);

template <>
InputParameters
validParams<PrescribedReactorPower>()
{
  InputParameters params = validParams<ReactorPower>();
  params.addRequiredParam<FunctionName>("function",
                                        "Function describing the reactor power over time");
  return params;
}

PrescribedReactorPower::PrescribedReactorPower(const InputParameters & parameters)
  : ReactorPower(parameters), _power_fn(getParam<FunctionName>("function"))
{
}

void
PrescribedReactorPower::addVariables()
{
  ReactorPower::addVariables();

  _const_power = !_sim.hasFunction(_power_fn);
  const Function & fn = _sim.getFunction(_power_fn);
  _sim.addConstantScalarIC(_power_var_name, fn.value(0, Point()));
}

void
PrescribedReactorPower::addMooseObjects()
{
  if (_const_power)
  {
    // we have to be able to control the power parameter, thus we need to setup a ConstantScalarAux
    // for it
    const Function & fn = _sim.getFunction(_power_fn);
    {
      std::string class_name = "ConstantScalarAux";
      InputParameters pars = _factory.getValidParams(class_name);
      pars.set<AuxVariableName>("variable") = _power_var_name;
      pars.set<Real>("value") = fn.value(0, Point());
      std::string nm = genName(name(), "power_aux");
      _sim.addAuxScalarKernel(class_name, nm, pars);
      connectObject(pars, nm, "power", "value");
    }
  }
  else
  {
    std::string class_name = "FunctionScalarAux";
    InputParameters pars = _factory.getValidParams(class_name);
    pars.set<AuxVariableName>("variable") = _power_var_name;
    pars.set<std::vector<FunctionName>>("function") = std::vector<FunctionName>(1, _power_fn);
    _sim.addAuxScalarKernel(class_name, genName(name(), "power_aux"), pars);
  }
}
