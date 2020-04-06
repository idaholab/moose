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
 * Userobject class to compute the equivalent scalar values for rank two tensor at individual
 * material point
 */
class NodalRankTwoScalarPD : public NodalRankTwoUserObjectBasePD
{
public:
  static InputParameters validParams();

  NodalRankTwoScalarPD(const InputParameters & parameters);

  virtual void gatherWeightedValue(unsigned int id, dof_id_type dof, Real dg_vol_frac) override;

protected:
  /**
   * Determines the information to be extracted from the tensor by using the
   * RankTwoScalarTools namespace, e.g., vonMisesStress, EquivalentPlasticStrain,
   * L2norm, MaxPrincipal eigenvalue, etc.
   */
  MooseEnum _scalar_type;

  /// Start point for axis used to calculate some cylinderical material tensor quantities
  const Point _point1;

  /// End point for axis used to calculate some cylinderical material tensor quantities
  const Point _point2;

  /// Direction vector used to calculate some material tensor quantities
  Point _input_direction;
};
