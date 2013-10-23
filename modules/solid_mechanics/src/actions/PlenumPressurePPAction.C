#include "PlenumPressurePPAction.h"

#include "Factory.h"
#include "FEProblem.h"
#include "Parser.h"

template<>
InputParameters validParams<PlenumPressurePPAction>()
{
  InputParameters params = validParams<Action>();

  params.addParam<Real>("initial_pressure", 0, "The initial pressure in the plenum.  If not given, a zero initial pressure will be used.");
  params.addParam<std::vector<PostprocessorName> >("material_input", "The name of the postprocessor(s) that holds the amount of material injected into the plenum.");
  params.addRequiredParam<Real>("R", "The universal gas constant for the units used.");
  params.addRequiredParam<PostprocessorName>("temperature", "The name of the average temperature postprocessor value.");
  params.addRequiredParam<PostprocessorName>("volume", "The name of the internal volume postprocessor value.");
  params.addParam<Real>("startup_time", 0, "The amount of time during which the pressure will ramp from zero to its true value.");
  params.addParam<std::string>("output_initial_moles", "", "The name to use when reporting the initial moles of gas.");
  params.addParam<std::string>("output", "The name to use for the plenum pressure value.");

  params.addParam<std::vector<Real> >("refab_time", "The time at which the plenum pressure must be reinitialized due to fuel rod refabrication.");
  params.addParam<std::vector<Real> >("refab_pressure", "The pressure of fill gas at refabrication.");
  params.addParam<std::vector<Real> >("refab_temperature", "The temperature at refabrication.");
  params.addParam<std::vector<Real> >("refab_volume", "The gas volume at refabrication.");
  params.addParam<std::vector<unsigned> >("refab_type", "The type of refabrication.  0 for instantaneous reset of gas, 1 for reset with constant fraction until next refabrication");

  return params;
}

PlenumPressurePPAction::PlenumPressurePPAction(const std::string & name, InputParameters params) :
  Action(name, params),
  _initial_pressure(getParam<Real>("initial_pressure")),
  _material_input(getParam<std::vector<PostprocessorName> >("material_input")),
  _R(getParam<Real>("R")),
  _temperature(getParam<PostprocessorName>("temperature")),
  _volume(getParam<PostprocessorName>("volume")),
  _startup_time(getParam<Real>("startup_time")),
  _output_initial_moles(getParam<std::string>("output_initial_moles")),

  _pp_name("PlenumPressurePostprocessor")
{
}

void
PlenumPressurePPAction::act()
{
  std::string name;
  if (isParamValid("output"))
  {
    name = getParam<std::string>("output");
  }
  else
  {
    std::string short_name(_name);
    // Chop off "BCs/PlenumPressure/"
    short_name.erase(0, 19);
    name = short_name;
  }


  InputParameters params = _factory.getValidParams(_pp_name);

  params.set<MooseEnum>("execute_on") = "residual";

  params.set<Real>("initial_pressure") = _initial_pressure;
  params.set<std::vector<PostprocessorName> >("material_input") = _material_input;
  params.set<Real>("R") = _R;
  params.set<PostprocessorName>("temperature") = _temperature;
  params.set<PostprocessorName>("volume") = _volume;
  params.set<Real>("startup_time") = _startup_time;
  params.set<std::string>("output_initial_moles") = _output_initial_moles;
  if (isParamValid("refab_time"))
  {
    params.set<std::vector<Real> >("refab_time") = getParam<std::vector<Real> >("refab_time");
  }
  if (isParamValid("refab_pressure"))
  {
    params.set<std::vector<Real> >("refab_pressure") = getParam<std::vector<Real> >("refab_pressure");
  }
  if (isParamValid("refab_temperature"))
  {
    params.set<std::vector<Real> >("refab_temperature") = getParam<std::vector<Real> >("refab_temperature");
  }
  if (isParamValid("refab_volume"))
  {
    params.set<std::vector<Real> >("refab_volume") = getParam<std::vector<Real> >("refab_volume");
  }
  if (isParamValid("refab_type"))
  {
    params.set<std::vector<unsigned> >("refab_type") = getParam<std::vector<unsigned> >("refab_type");
  }

  _problem->addPostprocessor(_pp_name, name, params);
}
