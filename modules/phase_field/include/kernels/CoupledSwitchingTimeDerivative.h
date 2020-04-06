//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "CoupledTimeDerivative.h"
#include "JvarMapInterface.h"
#include "DerivativeMaterialInterface.h"

// Forward Declaration

/**
 * This kernel adds a contribution
 * \f$ \left( \frac{\partial F_a}{\partial \eta_{ai}} f_a +
 *            \frac{\partial F_b}{\partial \eta_{ai}} f_b + ... \right)
 *    \frac{\partial \eta_{ai}}{\partial t} \f$
 * where \f$ a,b,.. \f$ are the phases, \f$ h_a, h_b,.. \f$ are the switching functions,
 * \f$ \eta_{ai} \f$ is the order parameter that is the nonlinear variable, \f$ t \f$ is time,
 * and \f$ F_a, F_b,.. \f$ are functions for each phase. For the grand-potential
 * model susceptibility equation, \f$ F_a \f$ etc. are the phase densities.
 */
class CoupledSwitchingTimeDerivative
  : public DerivativeMaterialInterface<JvarMapKernelInterface<CoupledTimeDerivative>>
{
public:
  static InputParameters validParams();

  CoupledSwitchingTimeDerivative(const InputParameters & parameters);
  virtual void initialSetup();

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  /// name of order parameter that derivatives are taken wrt (needed to retrieve
  /// the derivative material properties)
  const VariableName _v_name;

  /// Names of functions for each phase \f$ F_j \f$
  std::vector<MaterialPropertyName> _Fj_names;

  /// Number of phases
  const unsigned int _num_j;

  /// Values of the functions for each phase \f$ F_j \f$
  std::vector<const MaterialProperty<Real> *> _prop_Fj;

  /// Derivatives of the functions wrt the nonlinear variable for this kernel
  std::vector<const MaterialProperty<Real> *> _prop_dFjdv;

  /// Derivatives of the functions (needed for off-diagonal Jacobians)
  std::vector<std::vector<const MaterialProperty<Real> *>> _prop_dFjdarg;

  /// switching function names
  std::vector<MaterialPropertyName> _hj_names;

  /// Derivatives of the switching functions wrt the order parameter for this kernel
  std::vector<const MaterialProperty<Real> *> _prop_dhjdetai;

  /// Second derivatives of the switching functions wrt the order parameter for this kernel
  std::vector<const MaterialProperty<Real> *> _prop_d2hjdetai2;

  /// Second derivatives of the switching functions (needed for off-diagonal Jacobians)
  std::vector<std::vector<const MaterialProperty<Real> *>> _prop_d2hjdetaidarg;
};
