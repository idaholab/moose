/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef DERIVATIVESUMMATERIAL_H
#define DERIVATIVESUMMATERIAL_H

#include "DerivativeFunctionMaterialBase.h"

class DerivativeSumMaterial;

template <>
InputParameters validParams<DerivativeSumMaterial>();

class DerivativeSumMaterial : public DerivativeFunctionMaterialBase
{
public:
  DerivativeSumMaterial(const InputParameters & parameters);

  virtual void initialSetup();

protected:
  virtual void computeProperties();

  std::vector<std::string> _sum_materials;
  unsigned int _num_materials;

  /// arguments to construct a sum of the form \f$ c+\gamma\sum_iF_i \f$
  std::vector<Real> _prefactor;
  Real _constant;

  /// Function values of the summands.
  std::vector<const MaterialProperty<Real> *> _summand_F;

  /// Derivatives of the summands with respect to arg[i]
  std::vector<std::vector<const MaterialProperty<Real> *>> _summand_dF;

  /// Second derivatives of the summands.
  std::vector<std::vector<std::vector<const MaterialProperty<Real> *>>> _summand_d2F;

  /// Third derivatives of the summands.
  std::vector<std::vector<std::vector<std::vector<const MaterialProperty<Real> *>>>> _summand_d3F;
};

#endif // DERIVATIVESUMMATERIAL_H
