/*****************************************/
/* Written by andrew.wilkins@csiro.au    */
/* Please contact me if you make changes */
/*****************************************/

//  Holds maps between pressure_vars used in RichardsMaterial and the variable number used by MOOSE.
//
#include "RichardsPorepressureNames.h"

template<>
InputParameters validParams<RichardsPorepressureNames>()
{
  InputParameters params = validParams<GeneralUserObject>();
  params.addClassDescription("Holds information on the porepressure variable names");
  params.addRequiredCoupledVar("porepressure_vars", "List of variables that represent the porepressure.  In single-phase models you will just have one (eg \'pressure\'), in two-phase models you will have two (eg \'p_water p_gas\'), etc.  These names must also be used in your kernels.");
  return params;
}

RichardsPorepressureNames::RichardsPorepressureNames(const std::string & name, InputParameters parameters) :
  GeneralUserObject(name, parameters),
  Coupleable(parameters, false),
  ZeroInterface(parameters),
  _num_p(coupledComponents("porepressure_vars")),
  _the_names(std::string())

{
  unsigned int max_moose_var_num_seen(0);

  _moose_var_num.resize(_num_p);
  _moose_var_value.resize(_num_p);
  _moose_var_value_old.resize(_num_p);
  _moose_grad_var.resize(_num_p);
  for (unsigned int i=0 ; i<_num_p; ++i)
    {
      _moose_var_num[i] = coupled("porepressure_vars", i);
      max_moose_var_num_seen = (max_moose_var_num_seen > _moose_var_num[i] ? max_moose_var_num_seen : _moose_var_num[i]);
      _moose_var_value[i] = &coupledValue("porepressure_vars", i); // coupledValue returns a reference (an alias) to a VariableValue, and the & turns it into a pointer
      _moose_var_value_old[i] = (_is_transient ? &coupledValueOld("porepressure_vars", i) : &_zero);
      _moose_grad_var[i] = &coupledGradient("porepressure_vars", i);
      _the_names += getVar("porepressure_vars", i)->name() + " ";
    }
  _the_names.erase(_the_names.end() - 1, _the_names.end()); // remove trailing space

  _pressure_var_num.resize(max_moose_var_num_seen + 1);
  for (unsigned int i=0 ; i<max_moose_var_num_seen+1 ; ++i)
    _pressure_var_num[i] = _num_p; // NOTE: indicates that i is not a porepressure variable
  for (unsigned int i=0 ; i<_num_p; ++i)
    _pressure_var_num[_moose_var_num[i]] = i;
}

void
RichardsPorepressureNames::initialize()
{}

void
RichardsPorepressureNames::execute()
{}

void RichardsPorepressureNames::finalize()
{}



unsigned int
RichardsPorepressureNames::num_pp() const
{
  return _num_p;
}

unsigned int
RichardsPorepressureNames::moose_var_num(unsigned int pressure_var_num) const
{
  if (pressure_var_num >= _moose_var_num.size())
    mooseError("The pressure variable number " << pressure_var_num << " is out of bounds according to the RichardsPorepressureNames UserObject");
  return _moose_var_num[pressure_var_num];
}

unsigned int
RichardsPorepressureNames::pressure_var_num(unsigned int moose_var_num) const
{
  if (moose_var_num >= _pressure_var_num.size() || _pressure_var_num[moose_var_num] == _num_p)
    mooseError("The moose variable with number " << moose_var_num << " is not a porepressure according to the RichardsPoreporepressureNames UserObject");
  return _pressure_var_num[moose_var_num];
}

bool
RichardsPorepressureNames::not_pressure_var(unsigned int moose_var_num) const
{
  if (moose_var_num >= _pressure_var_num.size() || _pressure_var_num[moose_var_num] == _num_p)
    return true;
  return false;
}

std::string
RichardsPorepressureNames::pp_names() const
{
  return _the_names;
}

VariableValue *
RichardsPorepressureNames::pp_vals(unsigned int pressure_var_num) const
{
  return _moose_var_value[pressure_var_num]; // moose_var_value is a vector of pointers to VariableValuees
}

VariableValue *
RichardsPorepressureNames::pp_vals_old(unsigned int pressure_var_num) const
{
  return _moose_var_value_old[pressure_var_num];
}

VariableGradient *
RichardsPorepressureNames::grad_pp(unsigned int pressure_var_num) const
{
  return _moose_grad_var[pressure_var_num];
}


