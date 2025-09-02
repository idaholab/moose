//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "TotalLagrangianStressDivergence.h"
#include "HomogenizationInterface.h"

/// Total Lagrangian formulation with all homogenization terms (one disp_xyz field and macro_gradient scalar)
///
class HomogenizedTotalLagrangianStressDivergence
  : public HomogenizationInterface<TotalLagrangianStressDivergence>
{
public:
  static InputParameters validParams();
  HomogenizedTotalLagrangianStressDivergence(const InputParameters & parameters);

  /// Inform moose that this kernel covers the constraint scalar variable
  virtual std::set<std::string> additionalROVariables() override;

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

private:
  /// Indices for off-diagonal Jacobian components
  unsigned int _m, _n;

  /// Type of current homogenization constraint
  Homogenization::ConstraintType _ctype = Homogenization::ConstraintType::None;
};
