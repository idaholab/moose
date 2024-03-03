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

// Helpers common to the whole homogenization system
namespace HomogenizationB
{
/// Moose constraint type, for input
const MultiMooseEnum constraintType("strain stress none");
/// Constraint type: stress/PK stress or strain/deformation gradient
enum class ConstraintType
{
  Strain,
  Stress,
  None
};
typedef std::map<std::pair<unsigned int, unsigned int>, std::pair<ConstraintType, const Function *>>
    ConstraintMap;
}

/// Calculate the tensor corresponding to homogenization gradient
///
/// This class takes TWO scalar fields of the correct total size, representing a
/// constant deformation gradient over the domain, and casts them into
/// a RankTwo material property. _macro_gradientA is the 1st component and
/// _macro_gradient is the rest of the components.
///
class ComputeHomogenizedLagrangianStrainA : public Material
{
public:
  static InputParameters validParams();
  ComputeHomogenizedLagrangianStrainA(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;

protected:
  /// The base name for material properties
  const std::string _base_name;

  /// Constraint map
  HomogenizationB::ConstraintMap _cmap;

  /// ScalarVariable with the field
  const VariableValue & _macro_gradient;

  /// ScalarVariable with 1st component of the field
  const VariableValue & _macro_gradientA;

  /// Unwrapped into a tensor
  MaterialProperty<RankTwoTensor> & _homogenization_contribution;
};
