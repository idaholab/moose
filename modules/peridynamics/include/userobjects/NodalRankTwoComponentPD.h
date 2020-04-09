//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "NodalRankTwoUserObjectBasePD.h"

/**
 * Userobject class to compute the component values for rank two tensor at individual material point
 */
class NodalRankTwoComponentPD : public NodalRankTwoUserObjectBasePD
{
public:
  static InputParameters validParams();

  NodalRankTwoComponentPD(const InputParameters & parameters);

  virtual void gatherWeightedValue(unsigned int id, dof_id_type dof, Real dg_vol_frac) override;

protected:
  ///@{ Component indices
  const unsigned int _i;
  const unsigned int _j;
  ///@}
};
