/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

//  Holds maps between PorousFlow variables (porepressure, saturations) and the variable number used
//  by MOOSE.
#include "PorousFlowDictator.h"
#include "NonlinearSystem.h"

template <>
InputParameters
validParams<PorousFlowDictator>()
{
  InputParameters params = validParams<GeneralUserObject>();
  params.addClassDescription("Holds information on the PorousFlow variable names");
  params.addRequiredCoupledVar("porous_flow_vars",
                               "List of primary variables that are used in the PorousFlow "
                               "simulation.  Jacobian entries involving derivatives wrt these "
                               "variables will be computed.  In single-phase models you will just "
                               "have one (eg \'pressure\'), in two-phase models you will have two "
                               "(eg \'p_water p_gas\', or \'p_water s_water\'), etc.");
  params.addRequiredParam<unsigned int>("number_fluid_phases",
                                        "The number of fluid phases in the simulation");
  params.addRequiredParam<unsigned int>("number_fluid_components",
                                        "The number of fluid components in the simulation");
  return params;
}

PorousFlowDictator::PorousFlowDictator(const InputParameters & parameters)
  : GeneralUserObject(parameters),
    Coupleable(this, false),
    ZeroInterface(parameters),
    _num_variables(coupledComponents("porous_flow_vars")),
    _num_phases(getParam<unsigned int>("number_fluid_phases")),
    _num_components(getParam<unsigned int>("number_fluid_components"))
{
  _moose_var_num.resize(_num_variables);
  for (unsigned int i = 0; i < _num_variables; ++i)
    _moose_var_num[i] = coupled("porous_flow_vars", i);

  _pf_var_num.assign(_fe_problem.getNonlinearSystemBase().nVariables(),
                     _num_variables); // Note: the _num_variables assignment indicates that "this is
                                      // not a PorousFlow variable"
  for (unsigned int i = 0; i < _num_variables; ++i)
    if (_moose_var_num[i] < _pf_var_num.size())
      _pf_var_num[_moose_var_num[i]] = i;
    else
      // should not couple AuxVariables to the Dictator (Jacobian entries are not calculated for
      // them)
      mooseError("PorousFlowDictator: AuxVariables variables must not be coupled into the Dictator "
                 "for this is against specification #1984.  Variable number ",
                 i,
                 " is an AuxVariable.");
}

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
PorousFlowDictator::porousFlowVariableNum(unsigned int moose_var_num) const
{
  if (moose_var_num >= _pf_var_num.size() || _pf_var_num[moose_var_num] == _num_variables)
    mooseError("The Dictator proclaims that the moose variable with number ",
               moose_var_num,
               " is not a PorousFlow variable.  Exiting with error code 1984.");
  return _pf_var_num[moose_var_num];
}

bool
PorousFlowDictator::isPorousFlowVariable(unsigned int moose_var_num) const
{
  return !notPorousFlowVariable(moose_var_num);
}

bool
PorousFlowDictator::notPorousFlowVariable(unsigned int moose_var_num) const
{
  return moose_var_num >= _pf_var_num.size() || _pf_var_num[moose_var_num] == _num_variables;
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
