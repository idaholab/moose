//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MeshCut2DNucleationBase.h"
#include "RankTwoTensorForward.h"

class MeshCut2DRankTwoTensorNucleation : public MeshCut2DNucleationBase
{
public:
  static InputParameters validParams();

  MeshCut2DRankTwoTensorNucleation(const InputParameters & parameters);
  virtual ~MeshCut2DRankTwoTensorNucleation() {}

protected:
  /// The tensor from which the scalar quantity used as a nucleating criterion is extracted
  const MaterialProperty<RankTwoTensor> & _tensor;

  /// Threshold at which a crack is nucleated if exceeded
  const VariableValue & _nucleation_threshold;

  /// The type of scalar to be extracted from the tensor
  MooseEnum _scalar_type;

  /// Points used to define an axis of rotation for some scalar quantities
  const Point _point1;
  const Point _point2;

  /// Length of crack to be nucleated
  const Real _nucleation_length;

  /// Transformed Jacobian weights
  const MooseArray<Real> & _JxW;
  const MooseArray<Real> & _coord;

  virtual bool
  doesElementCrack(std::pair<RealVectorValue, RealVectorValue> & cutterElemNodes) override;
};
