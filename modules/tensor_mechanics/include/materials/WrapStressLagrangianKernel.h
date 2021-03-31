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
#include "DerivativeMaterialInterface.h"
#include "RankFourTensor.h"
#include "RankTwoTensor.h"

/// Wrap stresses to use the MOOSE materials with the Lagrangian kernels
//    This class wraps stress and jacobian information coming out of the
//    MOOSE material system to make it compatible with the
//    UpdatedLagrangianStressDivergence and TotalLagrangianStressDivergence
//    kernels
//
//    The object has two jobs:
//      1) Translate the stress coming from the MOOSE material to the
//         stress required by the Lagrangian kernels (Cauchy stress)
//      2) Update the material Jacobian coming from the MOOSE material to
//         make it the derivative of the Cauchy stress with respect to the
//         spatial velocity gradient
//
//    These tasks only matter for large deformations on the kernel side
//    if the kernel is just using small deformations this just copies over
//    the stress and tangent
//
class WrapStressLagrangianKernel : public DerivativeMaterialInterface<Material>
{
public:
  static InputParameters validParams();
  WrapStressLagrangianKernel(const InputParameters & parameters);
  virtual ~WrapStressLagrangianKernel(){};

protected:
  virtual void initQpStatefulProperties() override;
  virtual void computeQpProperties() override;

private:
  /// Do an objective integration of the small stresses provided by the material
  //    Right now this is based on the Truesdell rate, but we could provide
  //    options
  void _objectiveUpdate();

  /// Form the update tensor for the Truesdell rate
  RankFourTensor _truesdellUpdate(const RankTwoTensor & dL);
  /// Form the tangent tensor for the Truesdell rate
  RankFourTensor _truesdellTangent(const RankTwoTensor & S);

  /// Form the update tensor for a generic objective rate
  RankFourTensor _constructJ(const RankTwoTensor & Q);

protected:
  /// If true the kernel is using large displacements for the equilibrium
  bool _ld_kernel;
  /// If true the material provides the large deformation quantities directly
  bool _ld_material;

  /// The cauchy stress
  MaterialProperty<RankTwoTensor> & _cauchy_stress;
  /// The old cauchy stress (needed for objective integration)
  const MaterialProperty<RankTwoTensor> & _cauchy_stress_old;
  /// The derivative of the increment in the Cauchy stress wrt to the
  /// increment in the spatial velocity gradient
  MaterialProperty<RankFourTensor> & _material_jacobian;

  /// The inverse incremental deformation gradient
  const MaterialProperty<RankTwoTensor> & _df;

  /// Some stress measure coming from the material system
  const MaterialProperty<RankTwoTensor> & _stress_moose;
  /// Old stress from the material system
  const MaterialProperty<RankTwoTensor> & _stress_moose_old;
  /// The Jacobian coming from the material system
  const MaterialProperty<RankFourTensor> & _jacobian_moose;

  /// The 1st PK stress, postprocessed for the homogenization system to use
  MaterialProperty<RankTwoTensor> & _PK1;
  /// The volume change
  const MaterialProperty<Real> & _J;
  /// The inverse deformation gradient
  const MaterialProperty<RankTwoTensor> & _F_inv;
};
