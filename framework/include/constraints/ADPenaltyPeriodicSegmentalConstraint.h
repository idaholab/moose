//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADMortarScalarBase.h"
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
class ADPenaltyPeriodicSegmentalConstraint : public DerivativeMaterialInterface<ADMortarScalarBase>
{
public:
  static InputParameters validParams();
  ADPenaltyPeriodicSegmentalConstraint(const InputParameters & parameters);

protected:
  virtual void precalculateResidual() override;
  virtual void initScalarQpResidual() override;

  /**
   * Method for computing the residual at quadrature points
   */
  virtual ADReal computeQpResidual(Moose::MortarType mortar_type) override;

  /**
   * Method for computing the scalar part of residual at quadrature points
   */
  virtual ADReal computeScalarQpResidual() override;

  /**
   * Compute concentration jump before quadrature loop
   */
  void precalculateMaterial();

  /**
   * Compute penalty parameter
   */
  void precalculateStability();

protected:
  /// the temperature jump in global and interface coordinates;
  /// TM-analogy: _displacement_jump_global, _interface_displacement_jump
  ///@{
  ADReal _temp_jump_global;
  ///@}

  /// The stability parameter for the method
  ///@{
  Real _tau_s;
  ///@}

  /// (Pointer to) the controlled scalar variable
  const MooseVariableScalar * const _kappa_aux_ptr;

  /// Order of the homogenization variable, used in several places
  const unsigned int _ka_order;

  /// The controlled scalar variable
  const VariableValue & _kappa_aux;

  /// Input property from user as the value of the penalty parameter
  const Real _pen_scale;
};
