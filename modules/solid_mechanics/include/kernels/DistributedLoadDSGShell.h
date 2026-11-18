//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Kernel.h"
#include "RankTwoTensor.h"

class Function;

/**
 * DistributedLoadDSGShell applies a load per unit area of the mid-surface of a
 * DSG shell element, either along the axis of the displacement variable the
 * kernel acts on or along the normal of the element.
 */
class DistributedLoadDSGShell : public Kernel
{
public:
  static InputParameters validParams();

  DistributedLoadDSGShell(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;

  /// Returns the global axis of the variable of this kernel, its position in 'displacements'
  unsigned int displacementComponent();

  /// The displacement axis this kernel acts on, found from the variable of the kernel
  const unsigned int _component;

  /// Load per unit area, a function of time and position
  const Function & _function;

  /// Whether the load acts along the element normal instead of the displacement axis
  const bool _project_load_to_normal;

  /// Rotation matrix of the element frame, from the strain material
  const MaterialProperty<RankTwoTensor> & _rotation_matrix;
};
