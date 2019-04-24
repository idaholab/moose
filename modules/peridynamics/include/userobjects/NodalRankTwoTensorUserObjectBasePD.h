//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "NodalAuxVariableUserObjectBasePD.h"

class NodalRankTwoTensorUserObjectBasePD;

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
   * @param id   The local index of element node (either 1 or 2 for Edge2 element)
   * @param dof   The global DOF of element node
   * @param dgb_vol_sum   Summation of volume of all nodes used in calculation of bond-associated
   * deformation gradient for one neighbor
   * @param dgn_vol_sum   Summation of all bond-associated volumes for one node
   * @return What the function returns (if it returns anything)
   */
  virtual void
  gatherWeightedValue(unsigned int id, dof_id_type dof, Real dgb_vol_sum, Real dgn_vol_sum) = 0;

  /// Material properties tensor
  const MaterialProperty<RankTwoTensor> & _tensor;
};
