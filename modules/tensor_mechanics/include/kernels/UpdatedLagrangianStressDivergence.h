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

/// Enforce equilibrium with an updated Lagrangian formulation
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
//    use_displaced_mesh must be true for large deformation kinematic
//    The kernel enforces this with an error
//
class UpdatedLagrangianStressDivergence : public DerivativeMaterialInterface<Kernel>
{
public:
  static InputParameters validParams();
  UpdatedLagrangianStressDivergence(const InputParameters & parameters);
  virtual ~UpdatedLagrangianStressDivergence(){};

protected:
  /// Implement the R^{\alpha}=\int_{v}\sigma_{ij}\phi_{i,j}^{\alpha}dv residual
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

private:
  /// A component of the material Jacobian
  Real matJacobianComponent(const RankFourTensor & C,
                            unsigned int i,
                            unsigned int m,
                            const RealGradient & grad_psi,
                            const RealGradient & grad_phi,
                            const RankTwoTensor & df);
  /// A component of the geometric Jacobian
  Real geomJacobianComponent(unsigned int i,
                             unsigned int m,
                             const RealGradient & grad_psi,
                             const RealGradient & grad_phi,
                             const RankTwoTensor & stress);

protected:
  /// If true use large deformation kinematics
  bool _ld;

  /// Which component of the vector residual this kernel is responsible for
  unsigned int _component;
  /// Total number of displacements/size of residual vector
  unsigned int _ndisp;

  // The displacements and their gradients
  std::vector<unsigned int> _disp_nums;
  std::vector<MooseVariable *> _disp_vars;

  /// The Cauchy stress
  const MaterialProperty<RankTwoTensor> & _stress;
  /// The derivative of the increment in Cauchy stress wrt the increment in the
  /// spatial velocity gradient
  const MaterialProperty<RankFourTensor> & _material_jacobian;
  /// The inverse incremental deformation gradient
  const MaterialProperty<RankTwoTensor> & _df;
};
