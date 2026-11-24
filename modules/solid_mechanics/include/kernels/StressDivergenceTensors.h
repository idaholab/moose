//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ALEKernel.h"
#include "RankTwoTensor.h"
#include "RankFourTensor.h"
#include "JvarMapInterface.h"

// Forward Declarations

/**
 * StressDivergenceTensors mostly copies from StressDivergence.  There are small changes to use
 * RankFourTensor and RankTwoTensors instead of SymmElasticityTensors and SymmTensors.  This is done
 * to allow for more mathematical transparancy.
 */
class StressDivergenceTensors : public JvarMapKernelInterface<ALEKernel>
{
public:
  static InputParameters validParams();

  StressDivergenceTensors(const InputParameters & parameters);

  virtual void computeJacobian() override;
  virtual void computeOffDiagJacobian(unsigned int jvar) override;

protected:
  virtual void initialSetup() override;

  virtual void computeResidual() override;
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

  virtual void computeFiniteDeformJacobian();
  virtual void computeAverageGradientTest();
  virtual void computeAverageGradientPhi();

  /// Derivative of the stress wrt the inverse incremental deformation gradient at the current qp
  RankFourTensor finiteDeformStressDerivative(const RankTwoTensor & Fhat) const;
  /// Fill _locked_jacobian for the trial functions of displacement component coupled_component
  void computeLockedFiniteDeformJacobian(unsigned int coupled_component);

  /// Base name of the material system that this kernel applies to
  const std::string _base_name;

  bool _use_finite_deform_jacobian;

  /// The stress tensor that the divergence operator operates on
  const MaterialProperty<RankTwoTensor> & _stress;
  const MaterialProperty<RankFourTensor> & _Jacobian_mult;

  /// Per qp: the finite deformation Jacobian multiplier, or dstress/dFhat^-1 for _locked_jacobian
  std::vector<RankFourTensor> _finite_deform_Jacobian_mult;
  const MaterialProperty<RankTwoTensor> * _deformation_gradient;
  const MaterialProperty<RankTwoTensor> * _deformation_gradient_old;
  const MaterialProperty<RankTwoTensor> * _rotation_increment;
  /// Incremental deformation gradient before the strain material's volumetric locking correction
  const MaterialProperty<RankTwoTensor> * _unlocked_Fhat = nullptr;
  /// Blocks on which the strain material provides _unlocked_Fhat
  std::set<SubdomainID> _unlocked_Fhat_blocks;
  /// Whether the strain material of the current element applies the volumetric locking correction
  bool _material_lock = false;
  /// Whether the current Jacobian uses _locked_jacobian
  bool _use_locked_jacobian = false;
  /// Finite deformation Jacobian with volumetric locking per trial function and qp; row
  /// _component dotted with grad(test) gives the Jacobian entry
  std::vector<std::vector<RankTwoTensor>> _locked_jacobian;
  // MaterialProperty<RankTwoTensor> & _d_stress_dT;

  /// An integer corresponding to the direction this kernel acts in
  const unsigned int _component;

  /// Coupled displacement variables
  unsigned int _ndisp;

  /// Displacement variables IDs
  std::vector<unsigned int> _disp_var;

  /// eigen strain derivatives wrt coupled variables
  std::vector<std::vector<const MaterialProperty<RankTwoTensor> *>> _deigenstrain_dargs;

  const bool _out_of_plane_strain_coupled;
  const VariableValue * const _out_of_plane_strain;
  const unsigned int _out_of_plane_strain_var;
  const unsigned int _out_of_plane_direction;

  /// Whether this object is acting on the displaced mesh
  const bool _use_displaced_mesh;

  /// Gradient of test function averaged over the element. Used in volumetric locking correction calculation.
  std::vector<std::vector<Real>> _avg_grad_test;

  /// Gradient of phi function averaged over the element. Used in volumetric locking correction calculation.
  std::vector<std::vector<Real>> _avg_grad_phi;

  /// Flag for volumetric locking correction
  bool _volumetric_locking_correction;
};
