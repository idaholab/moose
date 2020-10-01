//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

//  Holds maps between Richards variables (porepressure, saturations) and the variable number used
//  by MOOSE.
//
#include "RichardsVarNames.h"

registerMooseObject("RichardsApp", RichardsVarNames);

InputParameters
RichardsVarNames::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params.addClassDescription("Holds information on the porepressure variable names");
  params.addRequiredCoupledVar("richards_vars",
                               "List of variables that represent the porepressures or "
                               "(porepressure, saturations).  In single-phase models you will just "
                               "have one (eg \'pressure\'), in two-phase models you will have two "
                               "(eg \'p_water p_gas\', or \'p_water s_water\', etc.  These names "
                               "must also be used in your kernels and material.");
  MooseEnum var_types("pppp", "pppp");
  params.addParam<MooseEnum>(
      "var_types",
      var_types,
      "Variable types for the problem.  Eg, 'pppp' means all the variables are pressure variables");

  return params;
}

RichardsVarNames::RichardsVarNames(const InputParameters & parameters)
  : GeneralUserObject(parameters),
    Coupleable(this, false),
    _num_v(coupledComponents("richards_vars")),
    _var_types(getParam<MooseEnum>("var_types")),
    _moose_var_num(coupledIndices("richards_vars")),
    _moose_var_value(coupledValues("richards_vars")),
    _moose_var_value_old(_is_transient ? coupledValuesOld("richards_vars")
                                       : std::vector<const VariableValue *>(_num_v, &_zero)),
    _moose_grad_var(coupledGradients("richards_vars"))
{
  unsigned int max_moose_var_num_seen = 0;

  _moose_nodal_var_value.resize(_num_v);
  _moose_nodal_var_value_old.resize(_num_v);
  for (unsigned int i = 0; i < _num_v; ++i)
  {
    max_moose_var_num_seen =
        (max_moose_var_num_seen > _moose_var_num[i] ? max_moose_var_num_seen : _moose_var_num[i]);
    _moose_nodal_var_value[i] = &coupledDofValues("richards_vars", i); // coupledDofValues returns
                                                                       // a reference (an alias) to
                                                                       // a VariableValue, and the
                                                                       // & turns it into a pointer
    _moose_nodal_var_value_old[i] =
        (_is_transient ? &coupledDofValuesOld("richards_vars", i) : &_zero);
  }

  _ps_var_num.resize(max_moose_var_num_seen + 1);
  for (unsigned int i = 0; i < max_moose_var_num_seen + 1; ++i)
    _ps_var_num[i] = _num_v; // NOTE: indicates that i is not a richards variable
  for (unsigned int i = 0; i < _num_v; ++i)
    _ps_var_num[_moose_var_num[i]] = i;
}

void
RichardsVarNames::initialize()
{
}

void
RichardsVarNames::execute()
{
}

void
RichardsVarNames::finalize()
{
}

unsigned int
RichardsVarNames::num_v() const
{
  return _num_v;
}

unsigned int
RichardsVarNames::richards_var_num(unsigned int moose_var_num) const
{
  if (moose_var_num >= _ps_var_num.size() || _ps_var_num[moose_var_num] == _num_v)
    mooseError("The moose variable with number ",
               moose_var_num,
               " is not a richards according to the RichardsVarNames UserObject");
  return _ps_var_num[moose_var_num];
}

bool
RichardsVarNames::not_richards_var(unsigned int moose_var_num) const
{
  if (moose_var_num >= _ps_var_num.size() || _ps_var_num[moose_var_num] == _num_v)
    return true;
  return false;
}

const VariableValue *
RichardsVarNames::richards_vals(unsigned int richards_var_num) const
{
  return _moose_var_value[richards_var_num]; // moose_var_value is a vector of pointers to
                                             // VariableValuees
}

const VariableValue *
RichardsVarNames::richards_vals_old(unsigned int richards_var_num) const
{
  return _moose_var_value_old[richards_var_num];
}

const VariableGradient *
RichardsVarNames::grad_var(unsigned int richards_var_num) const
{
  return _moose_grad_var[richards_var_num];
}

std::string
RichardsVarNames::var_types() const
{
  return _var_types;
}

const VariableValue *
RichardsVarNames::nodal_var(unsigned int richards_var_num) const
{
  return _moose_nodal_var_value[richards_var_num];
}

const VariableValue *
RichardsVarNames::nodal_var_old(unsigned int richards_var_num) const
{
  return _moose_nodal_var_value_old[richards_var_num];
}
