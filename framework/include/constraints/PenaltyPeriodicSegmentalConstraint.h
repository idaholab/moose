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

/**
 * This class enforces a periodic boundary condition between a microscale and macroscale field.
 * Target example for the Diffusion equation with isotropic, unitary diffusivity. Coupling is
 * made between a scalar macro-gradient variable and the temperature/concentration field within
 * the periodic domain. Primary and secondary surfaces are on opposing sides of the domain. Only
 * the macro to micro coupling terms are handled here. The micro-micro coupling terms are
 * handled using the PenaltyEqualValueConstraint applied to the same primary/secondary pair.
 *
 * The applied macroscale conjugate gradient is applied as `kappa_aux` vector as an auxillary
 * scalar. The computed macroscale gradient `kappa` is equal to this value for isotropic-unitary
 * diffusivity. The volume integral of the gradient of the primary field will be equal to these
 * imposed values.
 */

class PenaltyPeriodicSegmentalConstraint : public DerivativeMaterialInterface<MortarScalarBase>
{
public:
  static InputParameters validParams();
  PenaltyPeriodicSegmentalConstraint(const InputParameters & parameters);

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
                                              const unsigned int jvar) override;

  /**
   * Method for computing d-_kappa-residual / d-_var at quadrature points.
   */
  virtual Real computeScalarQpOffDiagJacobian(Moose::MortarType mortar_type,
                                              const unsigned int jvar) override;

  // Compute concentration jump
  void precalculateMaterial();
  // Compute penalty parameter
  void precalculateStability();

protected:
  /// the temperature jump in global and interface coordinates;
  /// TM-analogy: _displacement_jump_global, _interface_displacement_jump
  ///@{
  Real _temp_jump_global;
  ///@}

  /// The stability parameter for the method
  ///@{
  Real _tau_s;
  ///@}

  /// The controlled scalar variable ID
  const unsigned int _kappa_aux_var;

  /// Order of the homogenization variable, used in several places
  const unsigned int _ka_order;

  /// The controlled scalar variable
  const VariableValue & _kappa_aux;

  /// Input property from user as the value of the penalty parameter
  const Real _pen_scale;
};
