//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "NodalRankTwoTensorUserObjectBasePD.h"

class NodalRankTwoTensorComponentPD;

template <>
InputParameters validParams<NodalRankTwoTensorComponentPD>();

/**
 * Userobject class to compute the component values for rank two tensor at individual material point
 */
class NodalRankTwoTensorComponentPD : public NodalRankTwoTensorUserObjectBasePD
{
public:
  NodalRankTwoTensorComponentPD(const InputParameters & parameters);

  virtual void gatherWeightedValue(unsigned int id,
                                   dof_id_type dof,
                                   Real dgb_vol_sum,
                                   Real dgn_vol_sum) override;

protected:
  ///@{ Component indices
  const unsigned int _i;
  const unsigned int _j;
  ///@}
};
