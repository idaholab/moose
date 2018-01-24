//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef RANKTWOSCALARAUX_H
#define RANKTWOSCALARAUX_H

#include "AuxKernel.h"
#include "RankTwoTensor.h"

class RankTwoScalarAux;

template <>
InputParameters validParams<RankTwoScalarAux>();

/**
 * RankTwoScalarAux uses the namespace RankTwoScalarTools to compute scalar
 * values from Rank-2 tensors.
 */
class RankTwoScalarAux : public AuxKernel
{
public:
  RankTwoScalarAux(const InputParameters & parameters);

protected:
  virtual Real computeValue();

  const MaterialProperty<RankTwoTensor> & _tensor;

  /**
   * Determines the information to be extracted from the tensor by using the
   * RankTwoScalarTools namespace, e.g., vonMisesStress, EquivalentPlasticStrain,
   * L2norm, MaxPrincipal eigenvalue, etc.
   */
  MooseEnum _scalar_type;

  /// whether or not selected_qp has been set
  const bool _has_selected_qp;

  /// The std::vector will be evaluated at this quadpoint only if defined
  const unsigned int _selected_qp;

  const Point _point1;
  const Point _point2;
  Point _input_direction;
};

#endif // RANKTWOSCALARAUX_H
