//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ACBulk.h"
#include "ADAllenCahnBase.h"

/**
 * ACSwitching adds terms of the form
 * \f$ dh_a/d\eta_{ai} F_a + dh_b/d\eta_{ai} F_b + ... \f$
 * where \f$ a,b,.. \f$ are the phases, \f$ h_a, h_b,..\f$ are the switching functions,
 * \f$ \eta_{ai} is the order parameter for the phase/grain that is the nonlinear variable,
 * and \f$ F_a, F_b,.. \f$ are the free energies or grand potentials.
 */

template <bool is_ad>
using ACSwitchingBase = typename std::conditional<is_ad, ADAllenCahnBase<Real>, ACBulk<Real>>::type;

template <bool is_ad>
class ACSwitchingTempl : public ACSwitchingBase<is_ad>
{
public:
  static InputParameters validParams();

  ACSwitchingTempl(const InputParameters & parameters);
  virtual void initialSetup() override;

protected:
  /// name of order parameter that derivatives are taken wrt (needed to retrieve the derivative material properties)
  VariableName _etai_name;

  /// Names of free energy functions for each phase \f$ F_j \f$
  std::vector<MaterialPropertyName> _Fj_names;
  unsigned int _num_j;

  /// Values of the free energy functions for each phase \f$ F_j \f$
  std::vector<const GenericMaterialProperty<Real, is_ad> *> _prop_Fj;

  /// switching function names
  std::vector<MaterialPropertyName> _hj_names;

  /// Derivatives of the switching functions wrt the order parameter for this kernel
  std::vector<const GenericMaterialProperty<Real, is_ad> *> _prop_dhjdetai;

  using ACSwitchingBase<is_ad>::_qp;
  using ACSwitchingBase<is_ad>::_var;
  using ACSwitchingBase<is_ad>::_test;
};

class ACSwitching : public ACSwitchingTempl<false>
{
public:
  ACSwitching(const InputParameters & parameters);

  virtual void initialSetup() override;

protected:
  virtual Real computeDFDOP(PFFunctionType type) override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

  /// Derivatives of the free energy functions (needed for off-diagonal Jacobians)
  std::vector<std::vector<const MaterialProperty<Real> *>> _prop_dFjdarg;

  /// Second derivatives of the switching functions wrt the order parameter for this kernel
  std::vector<const MaterialProperty<Real> *> _prop_d2hjdetai2;

  /// Second derivatives of the switching functions (needed for off-diagonal Jacobians)
  std::vector<std::vector<const MaterialProperty<Real> *>> _prop_d2hjdetaidarg;
};

class ADACSwitching : public ACSwitchingTempl<true>
{
public:
  using ACSwitchingTempl<true>::ACSwitchingTempl;

protected:
  virtual ADReal computeDFDOP() override;
};
