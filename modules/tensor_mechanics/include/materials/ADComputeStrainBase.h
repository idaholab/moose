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
#include "ADRankTwoTensorForward.h"
#include "ADSymmetricRankTwoTensorForward.h"

/**
 * ADComputeStrainBase is the base class for strain tensors
 */
template <typename R2>
class ADComputeStrainBaseTempl : public Material
{
public:
  static InputParameters validParams();

  ADComputeStrainBaseTempl(const InputParameters & parameters);

  void initialSetup() override;

protected:
  virtual void initQpStatefulProperties() override;
  virtual void displacementIntegrityCheck();

  /// Coupled displacement variables
  const unsigned int _ndisp;

  /// Displacement variables
  std::vector<const ADVariableValue *> _disp;

  /// Gradient of displacements
  std::vector<const ADVariableGradient *> _grad_disp;

  /// Base name of the material system
  const std::string _base_name;

  ADMaterialProperty<R2> & _mechanical_strain;
  ADMaterialProperty<R2> & _total_strain;

  std::vector<MaterialPropertyName> _eigenstrain_names;
  std::vector<const ADMaterialProperty<R2> *> _eigenstrains;

  const ADMaterialProperty<R2> * const _global_strain;

  const bool _volumetric_locking_correction;
  const Real & _current_elem_volume;
};

typedef ADComputeStrainBaseTempl<RankTwoTensor> ADComputeStrainBase;
