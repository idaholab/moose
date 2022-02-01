//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Material.h"

/// Calculate the tensor corresponding to homogenization gradient
///
/// This class takes a scalar field of the right size representing a
/// constant deformation gradient over the domain and casts it into
/// a RankTwo material property
///
class ComputeHomogenizedLagrangianStrain : public Material
{
public:
  static InputParameters validParams();
  ComputeHomogenizedLagrangianStrain(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;

private:
  virtual RankTwoTensor calculateTensorContribution();

protected:
  /// Number of displacements
  const unsigned int _ndisp;

  /// Flag for large kinematics
  const bool _large_kinematics;

  /// ScalarVariable with the field
  const VariableValue & _macro_gradient;

  /// Unwrapped into a tensor
  MaterialProperty<RankTwoTensor> & _homogenization_contribution;
};
