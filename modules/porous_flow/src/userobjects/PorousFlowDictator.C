/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

//  Holds maps between PorousFlow variables (porepressure, saturations) and the variable number used by MOOSE.
#include "PorousFlowDictator.h"

template<>
InputParameters validParams<PorousFlowDictator>()
{
  InputParameters params = validParams<GeneralUserObject>();
  params.addClassDescription("Holds information on the PorousFlow variable names");
  params.addRequiredCoupledVar("porous_flow_vars", "List of primary variables that are used in the PorousFlow simulation.  Jacobian entries involving derivatives wrt these variables will be computed.  In single-phase models you will just have one (eg \'pressure\'), in two-phase models you will have two (eg \'p_water p_gas\', or \'p_water s_water\'), etc.  These names must also be used in your kernels and materials.");
  params.addRequiredParam<unsigned int>("number_fluid_phases", "The number of fluid phases in the simulation");
  params.addRequiredParam<unsigned int>("number_fluid_components", "The number of fluid components in the simulation");
  return params;
}

PorousFlowDictator::PorousFlowDictator(const InputParameters & parameters) :
    GeneralUserObject(parameters),
    Coupleable(this, false),
    ZeroInterface(parameters),
    _num_variables(coupledComponents("porous_flow_vars")),
    _num_phases(getParam<unsigned int>("number_fluid_phases")),
    _num_components(getParam<unsigned int>("number_fluid_components"))
{
  unsigned int max_moose_var_num_seen = 0;

  _moose_var_num.resize(_num_variables);
  for (unsigned int i = 0; i < _num_variables; ++i)
  {
    _moose_var_num[i] = coupled("porous_flow_vars", i);
    max_moose_var_num_seen = (max_moose_var_num_seen > _moose_var_num[i] ? max_moose_var_num_seen : _moose_var_num[i]);
  }

  _pf_var_num.resize(max_moose_var_num_seen + 1);
  for (unsigned int i = 0; i < max_moose_var_num_seen + 1; ++i)
    _pf_var_num[i] = _num_variables; // NOTE: indicates that i is not a richards variable
  for (unsigned int i = 0; i < _num_variables; ++i)
    _pf_var_num[_moose_var_num[i]] = i;
}

void
PorousFlowDictator::initialize()
{}

void
PorousFlowDictator::execute()
{}

void
PorousFlowDictator::finalize()
{}

unsigned int
PorousFlowDictator::numVariables() const
{
  return _num_variables;
}

unsigned int
PorousFlowDictator::numPhases() const
{
  return _num_phases;
}

unsigned int
PorousFlowDictator::numComponents() const
{
  return _num_components;
}

unsigned int
PorousFlowDictator::porflow_var_num(unsigned int moose_var_num) const
{
  if (moose_var_num >= _pf_var_num.size() || _pf_var_num[moose_var_num] == _num_variables)
    mooseError("The Dictator proclaims that the moose variable with number " << moose_var_num << ".  Exiting with error code 1984.");
  return _pf_var_num[moose_var_num];
}

bool
PorousFlowDictator::not_porflow_var(unsigned int moose_var_num) const
{
  if (moose_var_num >= _pf_var_num.size() || _pf_var_num[moose_var_num] == _num_variables)
    return true;
  return false;
}

const VariableName
PorousFlowDictator::pressureVariableNameDummy() const
{
  return "pressure_variable_dummy";
}

const VariableName
PorousFlowDictator::saturationVariableNameDummy() const
{
  return "saturation_variable_dummy";
}

const VariableName
PorousFlowDictator::temperatureVariableNameDummy() const
{
  return "temperature_variable_dummy";
}

const VariableName
PorousFlowDictator::massFractionVariableNameDummy() const
{
  return "mass_fraction_variable_dummy";
}
