/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef CRYSTALPLASTICITYSTATEVARIABLE_H
#define CRYSTALPLASTICITYSTATEVARIABLE_H

#include "CrystalPlasticityUOBase.h"

class CrystalPlasticityStateVariable;

template<>
InputParameters validParams<CrystalPlasticityStateVariable>();

/**
 * Crystal plasticity state variable userobject class.
 */
class CrystalPlasticityStateVariable : public CrystalPlasticityUOBase
{
public:
  CrystalPlasticityStateVariable(const InputParameters & parameters);

  virtual bool updateStateVariable(unsigned int qp, Real dt, std::vector<Real> & val) const;
  virtual void initSlipSysProps(std::vector<Real> & val) const;

protected:
  virtual void readFileInitSlipSysRes(std::vector<Real> & val) const;

  unsigned int _num_mat_state_var_evol_rate_comps;

  std::vector<const MaterialProperty<std::vector<Real> > * > _mat_prop_state_var_evol_rate_comps;

  const MaterialProperty<std::vector<Real> > &  _mat_prop_state_var;
  const MaterialProperty<std::vector<Real> > &  _mat_prop_state_var_old;

  /// File should contain initial values of the slip system resistances.
  std::string _slip_sys_res_prop_file_name;

  /// The hardening parameters in this class are read from .i file. The user can override to read from file.
  std::string _slip_sys_hard_prop_file_name;

  /// Read from options for initial values of internal variables
  MooseEnum _intvar_read_type;

  /// Numerical zero for internal variable
  Real _zero;

  /// Scale factor of individual component
  std::vector<Real> _scale_factor;
};

#endif // CRYSTALPLASTICITYSTATEVARIABLE_H
