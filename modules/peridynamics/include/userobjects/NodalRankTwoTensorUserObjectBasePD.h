//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef NODALRANKTWOTENSORUSEROBJECTBASEPD_H
#define NODALRANKTWOTENSORUSEROBJECTBASEPD_H

#include "NodalAuxVariableUserObjectBasePD.h"

class NodalRankTwoTensorUserObjectBasePD;
class RankTwoTensor;

template <>
InputParameters validParams<NodalRankTwoTensorUserObjectBasePD>();

/**
 * Base userobject class for rank two tensor at individual material point
 */
class NodalRankTwoTensorUserObjectBasePD : public NodalAuxVariableUserObjectBasePD
{
public:
  NodalRankTwoTensorUserObjectBasePD(const InputParameters & parameters);

  virtual void computeValue(unsigned int id, dof_id_type dof) override;

protected:
  /**
   * Function to gather bond-associated quantities to each material point
   */
  virtual void
  gatherWeightedValue(unsigned int id, dof_id_type dof, Real dgb_vol_sum, Real dgn_vol_sum) = 0;

  /// Rank two material properties to be fetch
  const MaterialProperty<RankTwoTensor> & _tensor;
};

#endif // NODALRANKTWOTENSORUSEROBJECTBASEPD_H
