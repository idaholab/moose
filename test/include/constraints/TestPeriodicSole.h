//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MortarScalarBase.h"
#include "DerivativeMaterialInterface.h"
#include "MooseVariableScalar.h"
#include "Assembly.h"

/// Test object to illustrate coupling between mortar spatial variables
/// and two separate scalar variables, kappa and kappa_other. The solution
/// is equivalent to PenaltyPeriodicSegmentalConstraint.
///
/// The kappa variable is split into two scalars: the first component called '_kappa'
/// herein and all other components called '_kappa_other' herein.
/// This decomposition is ONLY valid for 2D problems with a macro scalar vector [kappa_x kappa_y]
/// applied as a periodic boundary condition. For parameter _alpha = 0, the primary
/// scalar (_kappa) is kappa_x and the coupled scalar is kappa_y. For parameter _alpha = 1, the primary
/// scalar (_kappa) is kappa_y and the coupled scalar is kappa_x.
/// That mapping MUST be handled in the input file.
///
/// Thus, each instance of TestPeriodicSole acts on both primary/secondary field variables
/// and one scalar variable (_svar_alpha). The job of the constraint is to assemble the
/// residual and Jacobian terms arising from _svar_alpha (namely, selected entries).
/// It assembles one row of the Jacobian for the scalar variable as well as the couplings of that
/// scalar to the primary and secondary spatial variables, using logical checks with _alpha.
/// The entries for the other field/scalar variables are handled by the other instance of the
/// constraint, which have the opposite value of _alpha. The logical checks ensure the proper
/// decomposition of the jobs.
///
/// In summary, for x=disp_x etc. and k=kappa_x and a=kappa_y, then the contributions of the instances are
/// PenaltyEqualValueConstraint
/// R = [Rss+p, Rps+p, 0, 0]^T
/// J = [Jss, Jsp, 000, 000
///      Jps, Jpp, 000, 000
///      000, 000, 000, 000
///      000, 000, 000, 000]
/// TestPeriodicSole
/// _alpha=0
/// R = [Rsk, Rpk,  Rk,  00]^T
/// J = [000, 000, Jsk, 000
///      000, 000, Jpk, 000
///      Jks, Jkp, Jkk, Jka
///      000, 000, 000, 000]
/// _alpha=1
/// R = [Rsa, Rpa,  00,  Ra]^T
/// J = [000, 000, 000, Jsa
///      000, 000, 000, Jpa
///      000, 000, 000, 000
///      Jas, Jap, Jak, Jaa]
///
/// In this manner, the full R and J are obtained with NO duplication of jobs:
/// R = [Rs,  Rp,  Rk,  Ra ]^T
/// J = [Jss, Jsp, Jsk, Jsa
///      Jps, Jpp, Jpk, Jpa
///      Jks, Jkp, Jkk, Jka
///      Jas, Jap, Jak, Jaa]
///

class TestPeriodicSole : public DerivativeMaterialInterface<MortarScalarBase>
{
public:
  static InputParameters validParams();
  TestPeriodicSole(const InputParameters & parameters);

protected:
  virtual void precalculateResidual() override;
  virtual void precalculateJacobian() override;
  virtual void initScalarQpResidual() override;

  /**
   * Method for computing the residual at quadrature points
   */
  virtual Real computeQpResidual(Moose::MortarType mortar_type) override;
  Real computeQpJacobian(Moose::ConstraintJacobianType /*jacobian_type*/,
                         unsigned int /*jvar*/) override
  {
    return 0;
  };

  /**
   * Method for computing the scalar part of residual at quadrature points
   */
  virtual Real computeScalarQpResidual() override;

  /**
   * Method for computing the scalar variable part of Jacobian at
   * quadrature points
   */
  virtual Real computeScalarQpJacobian() override;

  // using MortarScalarBase::computeOffDiagJacobianScalar;

  /**
   * Method for computing d-_var-residual / d-_kappa at quadrature points.
   */
  virtual Real computeQpOffDiagJacobianScalar(Moose::MortarType mortar_type,
                                              const unsigned int svar_num) override;

  /**
   * Method for computing d-_kappa-residual / d-_var at quadrature points.
   */
  virtual Real computeScalarQpOffDiagJacobian(Moose::MortarType mortar_type,
                                              const unsigned int jvar_num) override;

  /**
   * Method for computing an off-diagonal jacobian component at quadrature points.
   */
  virtual Real computeScalarQpOffDiagJacobianScalar(const unsigned int svar_num) override;

  // Compute T jump and heat flux average/jump
  void precalculateMaterial();
  // Compute four stability tensors
  void precalculateStability();

protected:
  /// the temperature jump in global and interface coordiantes;
  /// TM-analogy: _displacement_jump_global, _interface_displacement_jump
  ///@{
  Real _temp_jump_global;
  ///@}

  /// The four stability parameters from the VMDG method
  ///@{
  Real _tau_s;
  ///@}

  /// The controlled scalar variable ID
  const unsigned int _kappa_aux_var;

  /// Order of the homogenization variable, used in several places
  const unsigned int _ka_order;

  /// The controlled scalar variable
  const VariableValue & _kappa_aux;

  /// Which component of the vector residual this constraint is responsible for
  const unsigned int _alpha;

  /// (Pointer to) Scalar variable this constraint operates on
  const MooseVariableScalar * const _kappao_var_ptr;

  /// The unknown scalar variable ID
  const unsigned int _kappao_var;

  /// Order of the scalar variable, used in several places
  const unsigned int _ko_order;

  /// Reference to the current solution at the current quadrature point
  const VariableValue & _kappa_other;

  const Real & _current_elem_volume;

  const Real & _current_side_volume;

  /// Input property to allow user modifying penalty parameter
  const Real _pen_scale;

private:
  /// hard code the penalty for now
  const Real pencoef = 1.0;
};
