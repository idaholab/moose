//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KKSACBulkBase.h"

// Forward Declarations

/**
 * KKSACBulkBase child class for the phase concentration difference term
 * \f$ \frac{dh}{d\eta}\frac{dF_a}{dc_a}(c_a-c_b) \f$
 * in the the Allen-Cahn bulk residual.
 *
 * The non-linear variable for this Kernel is the order parameter 'eta'.
 */
class KKSACBulkC : public KKSACBulkBase
{
public:
  static InputParameters validParams();

  KKSACBulkC(const InputParameters & parameters);

protected:
  virtual Real computeDFDOP(PFFunctionType type);
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  /// phase a concentration
  std::string _ca_name;
  unsigned int _ca_var;
  const VariableValue & _ca;

  /// phase b concentration
  std::string _cb_name;
  unsigned int _cb_var;
  const VariableValue & _cb;

  /// Derivative of the free energy function \f$ \frac d{dc_a} F_a \f$
  const MaterialProperty<Real> & _prop_dFadca;

  /// Second derivative of the free energy function \f$ \frac {d^2}{dc_a^2} F_a \f$
  const MaterialProperty<Real> & _prop_d2Fadca2;

  /// Mixed partial derivatives of the free energy function wrt ca and
  /// any other coupled variables \f$ \frac {d^2}{dc_a dq} F_a \f$
  std::vector<const MaterialProperty<Real> *> _prop_d2Fadcadarg;
};
