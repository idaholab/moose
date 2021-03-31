//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "DerivativeMaterialInterface.h"
#include "Kernel.h"
#include "RankFourTensor.h"
#include "RankTwoTensor.h"

#include "HomogenizationConstraintIntegral.h" // Index constants
#include "MooseVariableScalar.h"

/// Enforce equilibrium with a total Lagrangian formulation
//    This class enforces equilibrium when  used in conjunction with
//    the corresponding strain calculator (CalculateStrainLagrangianKernel)
//    and with either a stress calculator that provides the
//    "cauchy_stress" and the appropriate "material_jacobian",
//    which needs to be the derivative of the increment in Cauchy stress
//    with respect to the increment in the spatial velocity gradient.
//
//    The WrapStressLagrangianKernel exists to wrap the existing MOOSE
//    material system to provide this information, or new materials
//    can provide it directly.
//
//    The total Lagrangian formulation can interact with the homogenization
//    system defined by the HomogenizationConstraintScalarKernel and
//    HomogenizationConstraintIntegral user object by providing the
//    correct off-diagonal Jacobian entries.
//
class TotalLagrangianStressDivergence : public DerivativeMaterialInterface<Kernel>
{
public:
  static InputParameters validParams();
  TotalLagrangianStressDivergence(const InputParameters & parameters);
  virtual ~TotalLagrangianStressDivergence(){};

protected:
  ///
  virtual void initialSetup() override;

  /// Implement the
  /// R^{\alpha}=\int_{V}J\sigma_{ij}\phi_{i,K}^{\alpha}F_{Kj}^{-1}dV
  /// residual
  virtual Real computeQpResidual() override;
  /// On diagonal Jacobian, only involves the solid mechanics kernel
  virtual Real computeQpJacobian() override;
  /// Off diagonal Jacobian, solid mechanics + homogenization constraint
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;
  /// Homogenization constraint diagonal term
  virtual void computeOffDiagJacobianScalar(unsigned int jvar) override;

private:
  // *** Base kernel ***

  /// Large deformation residual where the deformation gradient isn't I
  Real largeDeformationResidual(const RealGradient & grad_phi);
  /// Small deformation residual without the large deformation kinematics
  Real smallDeformationResidual(const RealGradient & grad_phi);

  /// The material part of the Jacobian for small deformations
  Real smallDeformationMatJac(unsigned int i,
                              unsigned int k,
                              const RealGradient & grad_phi,
                              const RealGradient & grad_psi);
  /// The material part of the Jacobian for large deformations
  Real largeDeformationMatJac(unsigned int i,
                              unsigned int k,
                              const RealGradient & grad_phi,
                              const RealGradient & grad_psi);
  /// The geometric part of the Jacobian (only applies to large deformations)
  Real largeDeformationGeoJac(unsigned int i,
                              unsigned int k,
                              const RealGradient & grad_phi,
                              const RealGradient & grad_psi);

  // *** Homogenization-constraint system ***

  /// Calculate the displacement-scalar part of the off-diagonal constraint
  /// Jacobian
  Real computeBaseJacobian();
  /// Calculate the scalar-base part of the off-diagonal constraint
  /// Jacobian
  //    Properly this would belong in the ScalarKernel, but as it varies
  //    by element it's best to put it here
  Real computeConstraintJacobian();

  /// Calculate the small deformation displacement-scalar component
  Real sdBaseJacobian();
  /// Calculate the large deformation displacement-scalar Jacobian component
  Real ldBaseJacobian();

  /// Small deformation scalar-displacement component for strain constraints
  Real sdConstraintJacobianStrain();
  /// Small deformation scalar-displacement component for stress constraints
  Real sdConstraintJacobianStress();

  /// Large deformation scalar-displacement component for strain constraints
  Real ldConstraintJacobianStrain();
  /// Large deformation scalar-displacement component for stress constraints
  Real ldConstraintJacobianStress();

protected:
  /// If true use large kinematics
  bool _ld;

  /// Which residual vector index this kernel handles
  unsigned int _component;
  /// Total number of displacements
  unsigned int _ndisp;

  // The displacement variables and gradients
  std::vector<unsigned int> _disp_nums;
  std::vector<MooseVariable *> _disp_vars;
  std::vector<const VariableGradient *> _grad_disp;

  /// The Cauchy stress
  const MaterialProperty<RankTwoTensor> & _stress;
  /// The derivative of the Cauchy stress increment with respect to the
  /// spatial velocity gradient increment
  const MaterialProperty<RankFourTensor> & _material_jacobian;

  /// The inverse deformation gradient
  const MaterialProperty<RankTwoTensor> & _inv_def_grad;
  /// The determinant of the deformation gradient (volume change)
  const MaterialProperty<Real> & _detJ;
  /// The inverse incremental deformation gradient
  const MaterialProperty<RankTwoTensor> & _df;

  // The scalar variable used to enforce the homogenization constraints
  unsigned int _macro_gradient_num;
  const MooseVariableScalar * _macro_gradient;

  // Which indices are constrained and what types of constraints
  const HomogenizationConstants::index_list _indices;
  std::vector<HomogenizationConstants::ConstraintType> _ctypes;

  /// Used internally to iterate over each scalar component (i.e. each
  /// constraint)
  unsigned int _h;
};
