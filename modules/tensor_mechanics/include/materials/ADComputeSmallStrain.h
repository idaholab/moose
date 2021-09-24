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

/**
 * ADComputeSmallStrain defines a strain tensor, assuming small strains.
 */
template <typename R2>
class ADComputeSmallStrainTempl : public ADComputeStrainBaseTempl<R2>
{
public:
  static InputParameters validParams();

  ADComputeSmallStrainTempl(const InputParameters & parameters);

  virtual void computeProperties() override;

  using ADComputeStrainBaseTempl<R2>::_qp;
  using ADComputeStrainBaseTempl<R2>::_qrule;
  using ADComputeStrainBaseTempl<R2>::_eigenstrains;
  using ADComputeStrainBaseTempl<R2>::_total_strain;
  using ADComputeStrainBaseTempl<R2>::_global_strain;
  using ADComputeStrainBaseTempl<R2>::_mechanical_strain;
  using ADComputeStrainBaseTempl<R2>::_grad_disp;
  using ADComputeStrainBaseTempl<R2>::_JxW;
  using ADComputeStrainBaseTempl<R2>::_coord;
  using ADComputeStrainBaseTempl<R2>::_volumetric_locking_correction;
  using ADComputeStrainBaseTempl<R2>::_current_elem_volume;
};

typedef ADComputeSmallStrainTempl<RankTwoTensor> ADComputeSmallStrain;
typedef ADComputeSmallStrainTempl<SymmetricRankTwoTensor> ADSymmetricSmallStrain;
