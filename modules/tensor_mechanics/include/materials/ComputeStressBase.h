//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef COMPUTESTRESSBASE_H
#define COMPUTESTRESSBASE_H

#include "Material.h"
#include "RankTwoTensor.h"
#include "RankFourTensor.h"
#include "RotationTensor.h"
#include "DerivativeMaterialInterface.h"

class ComputeStressBase;

template <>
InputParameters validParams<ComputeStressBase>();

/**
 * ComputeStressBase is the base class for stress tensors
 */
class ComputeStressBase : public DerivativeMaterialInterface<Material>
{
public:
  ComputeStressBase(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties() override;
  virtual void computeQpProperties() override;
  virtual void computeQpStress() = 0;

  /**
   * InitialStress Deprecation: remove this method
   *
   * Adds initial stress, if it is provided, to _stress[_qp].  This
   * function should NOT be used if you calculate stress using
   *
   * stress = stress_old + elasticity * strain_increment
   *
   * because stress_old will already include initial stress.  However
   * this function SHOULD be used if your code uses
   *
   * stress = elasticity * (elastic_strain_old + strain_increment)
   * or
   * stress = elasticity * elastic_strain
   *
   * since in these cases the elastic_strain and elastic_strain_old
   * will not include any contribution from initial stress.
   */
  void addQpInitialStress();

  const std::string _base_name;
  const std::string _elasticity_tensor_name;

  const MaterialProperty<RankTwoTensor> & _mechanical_strain;
  MaterialProperty<RankTwoTensor> & _stress;
  MaterialProperty<RankTwoTensor> & _elastic_strain;

  const MaterialProperty<RankFourTensor> & _elasticity_tensor;

  /// Extra stress tensor
  const MaterialProperty<RankTwoTensor> & _extra_stress;

  /// initial stress components
  std::vector<Function *> _initial_stress_fcn;

  /// derivative of stress w.r.t. strain (_dstress_dstrain)
  MaterialProperty<RankFourTensor> & _Jacobian_mult;

  /// Parameter which decides whether to store old stress. This is required for HHT time integration and Rayleigh damping
  const bool _store_stress_old;

  /// Whether initial stress was provided.  InitialStress Deprecation: remove this.
  const bool _initial_stress_provided;

  /// Initial stress, if provided. InitialStress Deprecation: remove this.
  MaterialProperty<RankTwoTensor> * _initial_stress;

  /// Old value of initial stress, which is needed to correctly implement finite-strain rotations.  InitialStress Deprecation: remove this.
  const MaterialProperty<RankTwoTensor> * _initial_stress_old;
};

#endif // COMPUTESTRESSBASE_H
