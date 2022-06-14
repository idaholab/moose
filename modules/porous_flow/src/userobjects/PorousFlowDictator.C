//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowDictator.h"
#include "NonlinearSystem.h"

registerMooseObject("PorousFlowApp", PorousFlowDictator);

InputParameters
PorousFlowDictator::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
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
  params.addParam<unsigned int>("number_aqueous_equilibrium",
                                0,
                                "The number of secondary species in the aqueous-equilibrium "
                                "reaction system.  (Leave as zero if the simulation does not "
                                "involve chemistry)");
  params.addParam<unsigned int>("number_aqueous_kinetic",
                                0,
                                "The number of secondary species in the aqueous-kinetic reaction "
                                "system involved in precipitation and dissolution.  (Leave as zero "
                                "if the simulation does not involve chemistry)");
  params.addParam<unsigned int>("aqueous_phase_number",
                                0,
                                "The fluid phase number of the aqueous phase in which the "
                                "equilibrium and kinetic chemical reactions occur");
  return params;
}

PorousFlowDictator::PorousFlowDictator(const InputParameters & parameters)
  : GeneralUserObject(parameters),
    Coupleable(this, false),
    _num_variables(coupledComponents("porous_flow_vars")),
    _num_phases(getParam<unsigned int>("number_fluid_phases")),
    _num_components(getParam<unsigned int>("number_fluid_components")),
    _num_aqueous_equilibrium(getParam<unsigned int>("number_aqueous_equilibrium")),
    _num_aqueous_kinetic(getParam<unsigned int>("number_aqueous_kinetic")),
    _aqueous_phase_number(getParam<unsigned int>("aqueous_phase_number")),
    _consistent_fe_type(false),
    _fe_type(0)
{
  _moose_var_num.resize(_num_variables);
  for (unsigned int i = 0; i < _num_variables; ++i)
    _moose_var_num[i] = coupled("porous_flow_vars", i);

  if (_num_variables > 0)
  {
    _consistent_fe_type = true;
    _fe_type = FEType(getFieldVar("porous_flow_vars", 0)->feType());
    for (unsigned int i = 1; i < _num_variables; ++i)
      if (getFieldVar("porous_flow_vars", i)->feType() != _fe_type)
        _consistent_fe_type = false;
  }

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

  if ((_num_phases > 0) && (_aqueous_phase_number >= _num_phases))
    mooseError("PorousflowDictator: The aqueous phase number must be less than the number of fluid "
               "phases.  The Dictator does not appreciate jokes.");

  // Don't include permeabiity derivatives in the Jacobian by default (overwrite using
  // usePermDerivs()) when necessary in permeabiity material classes
  _perm_derivs = false;
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
PorousFlowDictator::numAqueousEquilibrium() const
{
  return _num_aqueous_equilibrium;
}

unsigned int
PorousFlowDictator::numAqueousKinetic() const
{
  return _num_aqueous_kinetic;
}

unsigned int
PorousFlowDictator::aqueousPhaseNumber() const
{
  return _aqueous_phase_number;
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

unsigned int
PorousFlowDictator::mooseVariableNum(unsigned int porous_flow_var_num) const
{
  if (porous_flow_var_num >= _num_variables)
    mooseError("The Dictator proclaims that there is no such PorousFlow variable with number ",
               porous_flow_var_num,
               ".  Exiting with error code 1984.");
  return _moose_var_num[porous_flow_var_num];
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

bool
PorousFlowDictator::consistentFEType() const
{
  return _consistent_fe_type;
}

FEType
PorousFlowDictator::feType() const
{
  return _fe_type;
}
