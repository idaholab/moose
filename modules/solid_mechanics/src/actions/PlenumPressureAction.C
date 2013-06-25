#include "PlenumPressureAction.h"

#include "Factory.h"
#include "FEProblem.h"
#include "Parser.h"

template<>
InputParameters validParams<PlenumPressureAction>()
{
  InputParameters params = validParams<Action>();
  params.addRequiredParam<std::vector<BoundaryName> >("boundary", "The list of boundary IDs from the mesh where the pressure will be applied");
  params.addRequiredParam<NonlinearVariableName>("disp_x", "The x displacement");
  params.addParam<NonlinearVariableName>("disp_y", "", "The y displacement");
  params.addParam<NonlinearVariableName>("disp_z", "", "The z displacement");

  params.addParam<std::vector<AuxVariableName> >("save_in_disp_x", "The save_in variables for x displacement");
  params.addParam<std::vector<AuxVariableName> >("save_in_disp_y", "The save_in variables for y displacement");
  params.addParam<std::vector<AuxVariableName> >("save_in_disp_z", "The save_in variables for z displacement");

  params.addParam<Real>("initial_pressure", 0, "The initial pressure in the plenum.  If not given, a zero initial pressure will be used.");
  params.addParam<std::vector<PostprocessorName> >("material_input", "The name of the postprocessor(s) that holds the amount of material injected into the plenum.");
  params.addRequiredParam<Real>("R", "The universal gas constant for the units used.");
  params.addRequiredParam<PostprocessorName>("temperature", "The name of the average temperature postprocessor value.");
  params.addRequiredParam<PostprocessorName>("volume", "The name of the internal volume postprocessor value.");
  params.addParam<Real>("startup_time", 0, "The amount of time during which the pressure will ramp from zero to its true value.");
  params.addParam<PostprocessorName>("output_initial_moles", "", "The reporting postprocessor to use for the initial moles of gas.");
  params.addParam<PostprocessorName>("output", "", "The reporting postprocessor to use for the plenum pressure value.");

  params.addParam<std::vector<Real> >("refab_time", "The time at which the plenum pressure must be reinitialized due to fuel rod refabrication.");
  params.addParam<std::vector<Real> >("refab_pressure", "The pressure of fill gas at refabrication.");
  params.addParam<std::vector<Real> >("refab_temperature", "The temperature at refabrication.");
  params.addParam<std::vector<Real> >("refab_volume", "The gas volume at refabrication.");
  params.addParam<std::vector<unsigned> >("refab_type", "The type of refabrication.  0 for instantaneous reset of gas, 1 for reset with constant fraction until next refabrication");

  return params;
}

PlenumPressureAction::PlenumPressureAction(const std::string & name, InputParameters params) :
  Action(name, params),
  _boundary(getParam<std::vector<BoundaryName> >("boundary")),
  _disp_x(getParam<NonlinearVariableName>("disp_x")),
  _disp_y(getParam<NonlinearVariableName>("disp_y")),
  _disp_z(getParam<NonlinearVariableName>("disp_z")),
  _initial_pressure(getParam<Real>("initial_pressure")),
  _material_input(getParam<std::vector<PostprocessorName> >("material_input")),
  _R(getParam<Real>("R")),
  _temperature(getParam<PostprocessorName>("temperature")),
  _volume(getParam<PostprocessorName>("volume")),
  _startup_time(getParam<Real>("startup_time")),
  _output_initial_moles(getParam<PostprocessorName>("output_initial_moles")),
  _output(getParam<PostprocessorName>("output")),

  _kernel_name("PlenumPressure"),
  _use_displaced_mesh(true)
{
  _save_in_vars.push_back(getParam<std::vector<AuxVariableName> >("save_in_disp_x"));
  _save_in_vars.push_back(getParam<std::vector<AuxVariableName> >("save_in_disp_y"));
  _save_in_vars.push_back(getParam<std::vector<AuxVariableName> >("save_in_disp_z"));

  _has_save_in_vars.push_back(params.isParamValid("save_in_disp_x"));
  _has_save_in_vars.push_back(params.isParamValid("save_in_disp_y"));
  _has_save_in_vars.push_back(params.isParamValid("save_in_disp_z"));
}

void
PlenumPressureAction::act()
{
  // Determine number of dimensions
  unsigned int dim(1);
  if (_disp_y != "")
  {
    ++dim;
  }
  if (_disp_z != "")
  {
    ++dim;
  }

  std::vector<NonlinearVariableName> vars;
  vars.push_back(_disp_x);
  vars.push_back(_disp_y);
  vars.push_back(_disp_z);
  std::string short_name(_name);
  // Chop off "BCs/PlenumPressure/"
  short_name.erase(0, 5+_kernel_name.size());
  for (unsigned int i(0); i < dim; ++i)
  {
    std::stringstream name;
    name << "BCs/";
    name << short_name;
    name << "_";
    name << i;

    InputParameters params = _factory.getValidParams(_kernel_name);

    params.set<std::vector<BoundaryName> >("boundary") = _boundary;

    params.set<Real>("initial_pressure") = _initial_pressure;
    params.set<std::vector<PostprocessorName> >("material_input") = _material_input;
    params.set<Real>("R") = _R;
    params.set<PostprocessorName>("temperature") = _temperature;
    params.set<PostprocessorName>("volume") = _volume;
    params.set<Real>("startup_time") = _startup_time;
    params.set<PostprocessorName>("output_initial_moles") = _output_initial_moles;
    params.set<PostprocessorName>("output") = _output;
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
    params.set<bool>("use_displaced_mesh") = _use_displaced_mesh;

    params.set<int>("component") = i;
    params.set<NonlinearVariableName>("variable") = vars[i];
    if (_has_save_in_vars[i])
    {
      params.set<std::vector<AuxVariableName> >("save_in") = _save_in_vars[i];
    }

    _problem->addBoundaryCondition(_kernel_name, name.str(), params);
  }
}
