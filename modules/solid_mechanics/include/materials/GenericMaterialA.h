//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ComputeEigenstrainBase.h"
#include "RankTwoTensor.h"
#include "RankFourTensor.h"

template <bool is_ad>
class GenericMaterialATempl : public ComputeEigenstrainBaseTempl<is_ad>
{

public:
  static InputParameters validParams();
  GenericMaterialATempl(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties() override;
  virtual void computeQpEigenstrain() override;

  ///Stores the total eigenstrain in the previous step
  const MaterialProperty<RankTwoTensor> & _eigenstrain_old;

private:
  const GenericMaterialProperty<RankTwoTensor, is_ad> & _mechanical_strain;
  const GenericMaterialProperty<RankTwoTensor, is_ad> & _total_strain;
  const GenericMaterialProperty<RankTwoTensor, is_ad> & _elastic_strain;

  using ComputeEigenstrainBaseTempl<is_ad>::_qp;
  using ComputeEigenstrainBaseTempl<is_ad>::_eigenstrain;
};

typedef GenericMaterialATempl<false> GenericMaterialA;
typedef GenericMaterialATempl<true> ADGenericMaterialA;
