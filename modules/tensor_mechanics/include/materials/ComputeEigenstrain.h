//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ComputeEigenstrainBase.h"

#include "RankTwoTensor.h"

/**
 * ComputeEigenstrain computes an Eigenstrain that is a function of a single variable defined by a
 * base tensor and a scalar function defined in a Derivative Material.
 */
template <bool is_ad>
class ComputeEigenstrainTempl : public ComputeEigenstrainBaseTempl<is_ad>
{
public:
  static InputParameters validParams();

  ComputeEigenstrainTempl(const InputParameters & parameters);

protected:
  virtual void computeQpEigenstrain() override;

  const GenericMaterialProperty<Real, is_ad> & _prefactor;

  RankTwoTensor _eigen_base_tensor;

  using Material::_qp;
  using ComputeEigenstrainBaseTempl<is_ad>::_eigenstrain;
};

typedef ComputeEigenstrainTempl<false> ComputeEigenstrain;
typedef ComputeEigenstrainTempl<true> ADComputeEigenstrain;
