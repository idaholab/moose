//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "CrystalPlasticityUOBase.h"

/**
 * Crystal plasticity state variable userobject class.
 */
class CrystalPlasticityStateVariable : public CrystalPlasticityUOBase
{
public:
  static InputParameters validParams();

  CrystalPlasticityStateVariable(const InputParameters & parameters);

  virtual bool updateStateVariable(unsigned int qp,
                                   Real dt,
                                   std::vector<Real> & val,
                                   std::vector<Real> & val_old) const;
  virtual void initSlipSysProps(std::vector<Real> & val, const Point & q_point) const;

protected:
  virtual void readInitialValueFromFile(std::vector<Real> & val) const;

  virtual void readInitialValueFromInline(std::vector<Real> & val) const;

  virtual void provideInitialValueByUser(std::vector<Real> & /*val*/,
                                         const Point & /*q_point*/) const;

  unsigned int _num_mat_state_var_evol_rate_comps;

  std::vector<const MaterialProperty<std::vector<Real>> *> _mat_prop_state_var_evol_rate_comps;

  const MaterialProperty<std::vector<Real>> & _mat_prop_state_var;

  /// File should contain initial values of the state variable.
  FileName _state_variable_file_name;

  /// Read from options for initial values of internal variables
  MooseEnum _intvar_read_type;

  /** The _groups variable is used to group slip systems and assign the initial values to each
   * group.
   *  The format is taken as [start end)
   *  i.e. _groups = '0 4 8 11', it means three groups 0-3, 4-7 and 8-11
   */
  std::vector<unsigned int> _groups;

  /** The _group_values are the initial values corresponding to each group.
   *  i.e. _groups = '0 4 8 11', and _group_values = '1.0 2.0 3.0'
   *  it means that initial values of slip system 0-3 is 1.0 , 4-7 is 2.0 and 8-11 is 3.0
   */
  std::vector<Real> _group_values;

  /// Numerical zero for internal variable
  Real _zero;

  /// Scale factor of individual component
  std::vector<Real> _scale_factor;
};
