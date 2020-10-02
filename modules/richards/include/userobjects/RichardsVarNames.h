//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralUserObject.h"
#include "Coupleable.h"

/**
 * This holds maps between pressure_var or pressure_var, sat_var
 * used in RichardsMaterial and kernels, etc, and the
 * variable number used internally by MOOSE
 */
class RichardsVarNames : public GeneralUserObject, public Coupleable
{
public:
  static InputParameters validParams();

  RichardsVarNames(const InputParameters & parameters);

  void initialize();
  void execute();
  void finalize();

  /// the number of porepressure variables
  unsigned int num_v() const;

  /**
   * the richards variable number
   * @param moose_var_num the MOOSE variable number
   * eg if richards_vars = 'pwater pgas', and the variables in
   * the simulation are 'temperature pwater pgas displacement'
   * then richards_var_num(2) = 1
   */
  unsigned int richards_var_num(unsigned int moose_var_num) const;

  /**
   * returns true if moose_var_num is not a richards var
   * @param moose_var_num the MOOSE variable number
   * eg if richards_vars = 'pwater pgas', and the variables in
   * the simulation are 'temperature pwater pgas displacement'
   * then not_pressure_var(0) = true, no_pressure_var(1) = false
   */
  bool not_richards_var(unsigned int moose_var_num) const;

  /**
   * a space-separated string of richards variable names
   * eg richards_names() = 'pwater pgas'
   */
  std::string richards_names() const;

  /**
   * a vector of pointers to VariableValues
   * @param richards_var_num the pressure variable number
   * eg if richards_vars = 'pwater pgas', then
   * (*richards_vals(1))[qp] = pgas evaluated at quadpoint qp
   * Also richards_vals(i) = &coupledValue
   */
  const VariableValue * richards_vals(unsigned int richards_var_num) const;

  /**
   * a vector of pointers to old VariableValues
   * @param richards_var_num the richards variable number
   * eg if richards_vars = 'pwater pgas', then
   * (*richards_vals_old(1))[qp] = old pgas evaluated at quadpoint qp
   * Also richards_vals_old(i) = &coupledValueOld
   */
  const VariableValue * richards_vals_old(unsigned int richards_var_num) const;

  /**
   * a vector of pointers to grad(Variable)
   * @param richards_var_num the richards variable number
   * eg if richards_vars = 'pwater pgas', then
   * (*grad_var(1))[qp] = grad(pgas) evaluated at quadpoint qp
   * Also grad_var(i) = &coupledGradient
   */
  const VariableGradient * grad_var(unsigned int richards_var_num) const;

  /**
   * The nodal variable values for the given richards_var_num
   * To extract a the value of pressure variable "pvar", at
   * node i, use (*RichardsVarNames.nodal_var(pvar))[i]
   * @param richards_var_num the richards variable number
   */
  const VariableValue * nodal_var(unsigned int richards_var_num) const;

  /**
   * The old nodal variable values for the given richards_var_num
   * @param richards_var_num the richards variable number
   */
  const VariableValue * nodal_var_old(unsigned int richards_var_num) const;

  /// return the _var_types string
  std::string var_types() const;

protected:
  /// number of richards variables
  unsigned int _num_v;

  /// physical meaning of the variables.  Eg 'pppp' means 'all variables are pressure variables'
  MooseEnum _var_types;

  /// _moose_var_num[i] = the moose variable number corresponding to richards variable i
  const std::vector<unsigned int> _moose_var_num;

  /// _pressure_var_num[i] = the richards variable corresponding to moose variable i
  std::vector<unsigned int> _ps_var_num;

  /// moose_var_value[i] = values of richards variable i
  const std::vector<const VariableValue *>
      _moose_var_value; // this is a vector of pointers to VariableValues

  /// moose_var_value_old[i] = old values of richards variable i
  const std::vector<const VariableValue *> _moose_var_value_old;

  /// moose_var_value[i] = values of richards variable i
  std::vector<const VariableValue *>
      _moose_nodal_var_value; // this is a vector of pointers to VariableValues

  /// moose_var_value_old[i] = old values of richards variable i
  std::vector<const VariableValue *> _moose_nodal_var_value_old;

  /// moose_grad_var[i] = gradient values of richards variable i
  const std::vector<const VariableGradient *> _moose_grad_var;
};
