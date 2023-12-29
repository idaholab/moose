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
#include "RankTwoTensor.h"
#include "RankFourTensor.h"
#include "RotationTensor.h"
#include "DerivativeMaterialInterface.h"

/**
 * ComputeStrainBase is the base class for strain tensors
 */
class ComputeStrainBase : public DerivativeMaterialInterface<Material>
{
public:
  static InputParameters validParams();

  ComputeStrainBase(const InputParameters & parameters);

  void initialSetup() override;

protected:
  virtual void initQpStatefulProperties() override;
  virtual void displacementIntegrityCheck();

  /// Coupled displacement variables
  unsigned int _ndisp;

  /// Displacement variables
  std::vector<const VariableValue *> _disp;

  /// Gradient of displacements
  std::vector<const VariableGradient *> _grad_disp;

  /// Base name of the material system
  const std::string _base_name;

  MaterialProperty<RankTwoTensor> & _mechanical_strain;

  MaterialProperty<RankTwoTensor> & _total_strain;

  std::vector<MaterialPropertyName> _eigenstrain_names;
  std::vector<const MaterialProperty<RankTwoTensor> *> _eigenstrains;

  const MaterialProperty<RankTwoTensor> * const _global_strain;

  const bool _volumetric_locking_correction;
  const Real & _current_elem_volume;
};
