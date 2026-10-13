//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ComputeEigenstrain.h"
#include "DerivativeMaterialInterface.h"

/**
 * Computes an eigenstrain whose scalar prefactor is supplied by a material property.
 *
 * The AD specialization obtains the prefactor and its AD derivatives through the AD material
 * property. The non-AD specialization obtains the prefactor derivatives from derivative material
 * properties and publishes the corresponding first and second derivatives of elastic_strain.
 */
template <bool is_ad>
class ComputeVariableEigenstrainTempl
  : public DerivativeMaterialInterface<ComputeEigenstrainTempl<is_ad>>
{
public:
  static InputParameters validParams();

  ComputeVariableEigenstrainTempl(const InputParameters & parameters);

protected:
  virtual void computeQpEigenstrain() override;

  /// Number of variables on which the prefactor material property depends
  const unsigned int _num_args;

  /// Names of the variables on which the prefactor material property depends
  std::vector<VariableName> _arg_names;

  /// First derivatives of the prefactor material property (non-AD only)
  std::vector<const MaterialProperty<Real> *> _dprefactor;

  /// Second derivatives of the prefactor material property (non-AD only)
  std::vector<std::vector<const MaterialProperty<Real> *>> _d2prefactor;

  /// First derivatives of elastic_strain with respect to args (non-AD only)
  std::vector<MaterialProperty<RankTwoTensor> *> _delastic_strain;

  /// Second derivatives of elastic_strain with respect to args (non-AD only)
  std::vector<std::vector<MaterialProperty<RankTwoTensor> *>> _d2elastic_strain;
};

typedef ComputeVariableEigenstrainTempl<false> ComputeVariableEigenstrain;
typedef ComputeVariableEigenstrainTempl<true> ADComputeVariableEigenstrain;
