//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PolycrystalDiffusivityTensorBase.h"
#include "DerivativeMaterialPropertyNameInterface.h"

/**
 * Calculates mobilities for grand potential model. The potential mobility (\chi*D)
 * is a tensor, while the Allen Cahn mobilities for the solid and void phases are
 * scalars.
 */
class GrandPotentialTensorMaterial : public PolycrystalDiffusivityTensorBase
{
public:
  static InputParameters validParams();

  GrandPotentialTensorMaterial(const InputParameters & parameters);

  virtual void computeProperties() override;

protected:
  /// mobility tensor
  std::string _chiD_name;
  MaterialProperty<RealTensorValue> & _chiD;
  MaterialProperty<RealTensorValue> * _dchiDdc;

  /// grain boundary mobility
  std::string _Ls_name;
  MaterialProperty<Real> & _Ls;

  /// void mobility
  std::string _Lv_name;
  MaterialProperty<Real> & _Lv;

  /// magnitude of mobility tensor
  MaterialProperty<Real> & _chiDmag;

  /// surface energy
  const MaterialProperty<Real> & _sigma_s;

  /// interface width
  Real _int_width;

  /// susceptibility
  const MaterialPropertyName _chi_name;
  const MaterialProperty<Real> & _chi;
  const MaterialProperty<Real> & _dchidc;
  std::vector<const MaterialProperty<Real> *> _dchideta;
  std::vector<MaterialProperty<RealTensorValue> *> _dchiDdeta;

  Real _GBMobility;
  Real _GBmob0;
  const Real _Q;

  /// solid phase order parameters
  std::vector<NonlinearVariableName> _vals_name;
};
