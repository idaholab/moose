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
    Coupleable(parameters, false),
    ZeroInterface(parameters),
    _num_variables(coupledComponents("porous_flow_vars")),
    _num_phases(getParam<unsigned int>("number_fluid_phases")),
    _num_components(getParam<unsigned int>("number_fluid_components"))
{
  unsigned int max_moose_var_num_seen = 0;

  _moose_var_num.resize(_num_variables);
  _moose_var_value.resize(_num_variables);
  _moose_var_value_old.resize(_num_variables);
  _moose_nodal_var_value.resize(_num_variables);
  _moose_nodal_var_value_old.resize(_num_variables);
  _moose_grad_var.resize(_num_variables);
  for (unsigned int i = 0; i < _num_variables; ++i)
  {
    _moose_var_num[i] = coupled("porous_flow_vars", i);
    max_moose_var_num_seen = (max_moose_var_num_seen > _moose_var_num[i] ? max_moose_var_num_seen : _moose_var_num[i]);
    _moose_var_value[i] = &coupledValue("porous_flow_vars", i); // coupledValue returns a reference (an alias) to a VariableValue, and the & turns it into a pointer
    _moose_var_value_old[i] = (_is_transient ? &coupledValueOld("porous_flow_vars", i) : &_zero);
    _moose_nodal_var_value[i] = &coupledNodalValue("porous_flow_vars", i); // coupledNodalValue returns a reference (an alias) to a VariableValue, and the & turns it into a pointer
    _moose_nodal_var_value_old[i] = (_is_transient ? &coupledNodalValueOld("porous_flow_vars", i) : &_zero);
    _moose_grad_var[i] = &coupledGradient("porous_flow_vars", i);
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

void PorousFlowDictator::finalize()
{}

unsigned int
PorousFlowDictator::num_variables() const
{
  return _num_variables;
}

unsigned int
PorousFlowDictator::num_phases() const
{
  return _num_phases;
}

unsigned int
PorousFlowDictator::num_components() const
{
  return _num_components;
}

unsigned int
PorousFlowDictator::porflow_var_num(unsigned int moose_var_num) const
{
  if (moose_var_num >= _pf_var_num.size() || _pf_var_num[moose_var_num] == _num_variables)
    mooseError("The moose variable with number " << moose_var_num << " is not a PorousFlow variable according to the PorousFlowDictator UserObject");
  return _pf_var_num[moose_var_num];
}

bool
PorousFlowDictator::not_porflow_var(unsigned int moose_var_num) const
{
  if (moose_var_num >= _pf_var_num.size() || _pf_var_num[moose_var_num] == _num_variables)
    return true;
  return false;
}

const VariableValue *
PorousFlowDictator::porflow_vals(unsigned int porflow_var_num) const
{
  return _moose_var_value[porflow_var_num]; // moose_var_value is a vector of pointers to VariableValuees
}

const VariableValue *
PorousFlowDictator::porflow_vals_old(unsigned int porflow_var_num) const
{
  return _moose_var_value_old[porflow_var_num];
}

const VariableGradient *
PorousFlowDictator::grad_var(unsigned int porflow_var_num) const
{
  return _moose_grad_var[porflow_var_num];
}

const VariableValue *
PorousFlowDictator::nodal_var(unsigned int porflow_var_num) const
{
  return _moose_nodal_var_value[porflow_var_num];
}

const VariableValue *
PorousFlowDictator::nodal_var_old(unsigned int porflow_var_num) const
{
  return _moose_nodal_var_value_old[porflow_var_num];
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
