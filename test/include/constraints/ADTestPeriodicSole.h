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

/// Test object to illustrate coupling between mortar spatial variables
/// and two separate scalar variables, kappa and kappa_other. The solution
/// is equivalent to PenaltyPeriodicSegmentalConstraint. AD-version.
///

class ADTestPeriodicSole : public DerivativeMaterialInterface<ADMortarScalarBase>
{
public:
  static InputParameters validParams();
  ADTestPeriodicSole(const InputParameters & parameters);

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

  // Compute T jump and heat flux average/jump
  void precalculateMaterial();
  // Compute four stability tensors
  void precalculateStability();

protected:
  /// the temperature jump in global and interface coordiantes;
  /// TM-analogy: _displacement_jump_global, _interface_displacement_jump
  ///@{
  ADReal _temp_jump_global;
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
  const ADVariableValue & _kappa_other;

  const Real & _current_elem_volume;

  const Real & _current_side_volume;

  /// Input property to allow user modifying penalty parameter
  const Real _pen_scale;

private:
  /// hard code the penalty for now
  const Real pencoef = 1.0;
};
