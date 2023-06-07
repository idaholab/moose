//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MeshCutNucleationBase.h"
#include "RankTwoTensorForward.h"

class MeshCutRankTwoTensorNucleation : public MeshCutNucleationBase
{
public:
  static InputParameters validParams();

  MeshCutRankTwoTensorNucleation(const InputParameters & parameters);
  virtual ~MeshCutRankTwoTensorNucleation() {}

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

  /// Whether to average the value for all quadrature points in an element
  bool _average;

  /// Length of crack to be nucleated
  const Real _nucleation_length;

  /// Transformed Jacobian weights
  const MooseArray<Real> & _JxW;
  const MooseArray<Real> & _coord;

  virtual bool
  doesElementCrack(std::pair<RealVectorValue, RealVectorValue> & cutterElemNodes) override;
};
