//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "NodalPatchRecovery.h"
#include "RankTwoTensor.h"

/**
 * RankTwoAux is designed to take the data in the RankTwoTensor material
 * property, for example stress or strain, and output the value for the
 * supplied indices.
 */

template <bool is_ad>
class RankTwoAuxTempl : public NodalPatchRecovery
{
public:
  static InputParameters validParams();

  RankTwoAuxTempl(const InputParameters & parameters);

protected:
  virtual Real computeValue();

private:
  const GenericMaterialProperty<RankTwoTensor, is_ad> & _tensor;
  const unsigned int _i;
  const unsigned int _j;

  /// whether or not selected_qp has been set
  const bool _has_selected_qp;

  /// The std::vector will be evaluated at this quadpoint only if defined
  const unsigned int _selected_qp;
};

typedef RankTwoAuxTempl<false> RankTwoAux;
typedef RankTwoAuxTempl<true> ADRankTwoAux;
