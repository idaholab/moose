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

/// Base class for implementing Variational Multiscale Nitsche (VMN)
/// Thermal (T) mechanics periodic boundary conditions 2D, and 3D.
/// Constructed in the spirit of the "Heat Diffusion" kernel.
///
/// This class provides the base for computing the residual that couples
/// the macro heat flux and the boundary temperature jump, and the off-
/// diagonal Jacobian terms
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

  /// (Pointer to) Scalar variable this kernel operates on
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
