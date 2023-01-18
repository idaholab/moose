//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "TotalLagrangianStressDivergenceS.h"

// Helpers common to the whole homogenization system
namespace HomogenizationS
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

/// Total Lagrangian formulation with all homogenization terms (one disp_xyz field and macro_gradient scalar)
///
class HomogenizedTotalLagrangianStressDivergenceS : public TotalLagrangianStressDivergenceS
{
public:
  static InputParameters validParams();
  HomogenizedTotalLagrangianStressDivergenceS(const InputParameters & parameters);

protected:
  /**
   * Method for computing the scalar part of residual for _kappa
   */
  virtual void computeScalarResidual() override;

  /**
   * Method for computing the scalar variable part of Jacobian for d-_kappa-residual / d-_kappa
   */
  virtual void computeScalarJacobian() override;

  /**
   * Method for computing an off-diagonal jacobian component d-_kappa-residual / d-jvar.
   * jvar is looped over all field variables, which herein is just disp_x and disp_y
   */
  virtual void computeScalarOffDiagJacobian(const unsigned int jvar_num) override;

  /**
   * Method for computing an off-diagonal jacobian component at quadrature points.
   */
  virtual Real computeScalarQpOffDiagJacobian(const unsigned int jvar_num) override;

  /**
   * Method for computing an off-diagonal jacobian component d-_var-residual / d-svar.
   * svar is looped over all scalar variables, which herein is just _kappa
   */
  virtual void computeOffDiagJacobianScalarLocal(const unsigned int svar_num) override;

  /**
   * Method for computing d-_var-residual / d-svar at quadrature points.
   */
  virtual Real computeQpOffDiagJacobianScalar(const unsigned int svar_num) override;

protected:
  /// Type of each constraint (stress or strain) for each component
  HomogenizationS::ConstraintMap _cmap;

  /// The constraint type; initialize with 'none'
  HomogenizationS::ConstraintType _ctype = HomogenizationS::ConstraintType::None;

  /// Used internally to iterate over each scalar component
  unsigned int _m;
  unsigned int _n;
  unsigned int _a;
  unsigned int _b;
};
