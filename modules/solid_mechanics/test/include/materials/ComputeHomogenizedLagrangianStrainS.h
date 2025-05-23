//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Material.h"

// Helpers common to the whole homogenization system
namespace HomogenizationM
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
/// This class takes a scalar field of the right size representing a
/// constant deformation gradient over the domain and casts it into
/// a RankTwo material property
///
class ComputeHomogenizedLagrangianStrainS : public Material
{
public:
  static InputParameters validParams();
  ComputeHomogenizedLagrangianStrainS(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;

protected:
  /// The base name for material properties
  const std::string _base_name;

  /// Constraint map
  HomogenizationM::ConstraintMap _cmap;

  /// ScalarVariable with the field
  const VariableValue & _macro_gradient;

  /// Unwrapped into a tensor
  MaterialProperty<RankTwoTensor> & _homogenization_contribution;
};
