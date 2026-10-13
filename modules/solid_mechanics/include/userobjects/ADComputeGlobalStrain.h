//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details

#pragma once

#include "ADMaterial.h"
#include "ADRankTwoTensorForward.h"

#include <utility>
#include <vector>

class GlobalStrainPeriodicDirUserObject;

/**
 * Builds the symmetric AD global-strain tensor from separate diagonal and
 * off-diagonal scalar variables.
 *
 * Active diagonal components satisfy periodic(i). Active off-diagonal
 * components satisfy periodic(i) || periodic(j).
 */
class ADComputeGlobalStrain : public ADMaterial
{
public:
  static InputParameters validParams();

  ADComputeGlobalStrain(const InputParameters & parameters);

protected:
  void initQpStatefulProperties() override;
  void computeQpProperties() override;

  void assignComponentIndices();
  void validateScalarVariables() const;
  void fillGlobalStrainTensor(ADRankTwoTensor & strain) const;

  /// Optional prefix for mechanics material properties.
  const std::string _base_name;

  /// Scalar variable holding xx, yy, and/or zz for periodic directions.
  const ADVariableValue & _diagonal_global_strain;

  /// Scalar variable holding active components in the order xy, xz, yz.
  /// This is null only when the problem has no active off-diagonal component.
  const ADVariableValue * const _off_diagonal_global_strain;

  const unsigned int _number_of_diagonal_components;
  const unsigned int _number_of_off_diagonal_components;

  ADMaterialProperty<RankTwoTensor> & _global_strain;

  const GlobalStrainPeriodicDirUserObject & _periodicity_uo;
  const VectorValue<bool> & _periodic_dir;

  std::vector<std::pair<unsigned int, unsigned int>> _diagonal_components;
  std::vector<std::pair<unsigned int, unsigned int>> _off_diagonal_components;

  const unsigned int _dim;
  const unsigned int _ndisp;
};
