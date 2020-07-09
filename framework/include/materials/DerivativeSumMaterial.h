//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "DerivativeFunctionMaterialBase.h"

template <bool is_ad>
class DerivativeSumMaterialTempl : public DerivativeFunctionMaterialBaseTempl<is_ad>
{
public:
  static InputParameters validParams();

  DerivativeSumMaterialTempl(const InputParameters & parameters);

  virtual void initialSetup();

protected:
  usingDerivativeFunctionMaterialBaseMembers(is_ad);

  virtual void computeProperties();

  std::vector<std::string> _sum_materials;
  unsigned int _num_materials;

  /// arguments to construct a sum of the form \f$ c+\gamma\sum_iF_i \f$
  std::vector<Real> _prefactor;
  Real _constant;

  /// Flag to optionally turn on or off validateCoupling
  const bool _validate_coupling;

  /// Function values of the summands.
  std::vector<const GenericMaterialProperty<Real, is_ad> *> _summand_F;

  /// Derivatives of the summands with respect to arg[i]
  std::vector<std::vector<const GenericMaterialProperty<Real, is_ad> *>> _summand_dF;

  /// Second derivatives of the summands.
  std::vector<std::vector<std::vector<const GenericMaterialProperty<Real, is_ad> *>>> _summand_d2F;

  /// Third derivatives of the summands.
  std::vector<std::vector<std::vector<std::vector<const GenericMaterialProperty<Real, is_ad> *>>>>
      _summand_d3F;
};

typedef DerivativeSumMaterialTempl<false> DerivativeSumMaterial;
typedef DerivativeSumMaterialTempl<true> ADDerivativeSumMaterial;
