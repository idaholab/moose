//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Material.h"

#include "RankTwoTensor.h"
#include "SymmetricRankTwoTensor.h"

#include "RankFourTensor.h"
#include "SymmetricRankFourTensor.h"

template <typename T, bool symmetrize>
struct MandelConversion
{
  using from = std::false_type;
  using to = std::false_type;
};
template <>
struct MandelConversion<RankTwoTensor, true>
{
  using from = RankTwoTensor;
  using to = SymmetricRankTwoTensor;
};
template <>
struct MandelConversion<RankTwoTensor, false>
{
  using from = SymmetricRankTwoTensor;
  using to = RankTwoTensor;
};
template <>
struct MandelConversion<RankFourTensor, true>
{
  using from = RankFourTensor;
  using to = SymmetricRankFourTensor;
};
template <>
struct MandelConversion<RankFourTensor, false>
{
  using from = SymmetricRankFourTensor;
  using to = RankFourTensor;
};

/**
 * Convert tensorial material properties between Mandel notation and full notation
 */
template <typename T, bool symmetrize>
class MandelConverter : public Material
{
public:
  static InputParameters validParams();

  MandelConverter(const InputParameters & parameters);

  using FromType = typename MandelConversion<T, symmetrize>::from;
  using ToType = typename MandelConversion<T, symmetrize>::to;

protected:
  void initQpStatefulProperties() override;
  void computeQpProperties() override;

  const MaterialProperty<FromType> & _from;
  MaterialProperty<ToType> & _to;
};

using RankTwoTensorToSymmetricRankTwoTensor = MandelConverter<RankTwoTensor, true>;
using SymmetricRankTwoTensorToRankTwoTensor = MandelConverter<RankTwoTensor, false>;
using RankFourTensorToSymmetricRankFourTensor = MandelConverter<RankFourTensor, true>;
using SymmetricRankFourTensorToRankFourTensor = MandelConverter<RankFourTensor, false>;
