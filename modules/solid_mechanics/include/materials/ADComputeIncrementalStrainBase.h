//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADComputeStrainBase.h"

#define usingComputeIncrementalStrainBaseMembers                                                   \
  usingComputeStrainBaseMembers;                                                                   \
  using ADComputeIncrementalStrainBaseTempl<R2>::_grad_disp_old;                                   \
  using ADComputeIncrementalStrainBaseTempl<R2>::_strain_rate;                                     \
  using ADComputeIncrementalStrainBaseTempl<R2>::_strain_increment;                                \
  using ADComputeIncrementalStrainBaseTempl<R2>::_rotation_increment;                              \
  using ADComputeIncrementalStrainBaseTempl<R2>::_mechanical_strain_old;                           \
  using ADComputeIncrementalStrainBaseTempl<R2>::_total_strain_old;                                \
  using ADComputeIncrementalStrainBaseTempl<R2>::_eigenstrains_old

/**
 * ADComputeIncrementalStrainBase is the base class for strain tensors using incremental
 * formulations
 */
template <typename R2>
class ADComputeIncrementalStrainBaseTempl : public ADComputeStrainBaseTempl<R2>
{
public:
  static InputParameters validParams();

  ADComputeIncrementalStrainBaseTempl(const InputParameters & parameters);

  void initialSetup() override;

protected:
  using ADR2 = Moose::GenericType<R2, true>;

  virtual void initQpStatefulProperties() override;

  void subtractEigenstrainIncrementFromStrain(ADR2 & strain);

  std::vector<const VariableGradient *> _grad_disp_old;

  ADMaterialProperty<R2> & _strain_rate;
  ADMaterialProperty<R2> & _strain_increment;
  ADMaterialProperty<RankTwoTensor> & _rotation_increment;

  const MaterialProperty<R2> & _mechanical_strain_old;
  const MaterialProperty<R2> & _total_strain_old;

  std::vector<const MaterialProperty<R2> *> _eigenstrains_old;

  usingComputeStrainBaseMembers;
};

typedef ADComputeIncrementalStrainBaseTempl<RankTwoTensor> ADComputeIncrementalStrainBase;
typedef ADComputeIncrementalStrainBaseTempl<SymmetricRankTwoTensor>
    ADSymmetricIncrementalStrainBase;
