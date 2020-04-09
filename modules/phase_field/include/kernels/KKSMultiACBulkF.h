//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KKSMultiACBulkBase.h"

// Forward Declarations

/**
 * KKSMultiACBulkBase child class for the free energy term
 * \f$ \sum_j \frac{\partial h_j}{\partial \eta_i} F_j + w_i \frac{dg}{d\eta_i} \f$
 * in the the Allen-Cahn bulk residual.
 *
 * The non-linear variable for this Kernel is the order parameter \f$ eta_i \f$.
 */
class KKSMultiACBulkF : public KKSMultiACBulkBase
{
public:
  static InputParameters validParams();

  KKSMultiACBulkF(const InputParameters & parameters);

protected:
  virtual Real computeDFDOP(PFFunctionType type);
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  /// double well height parameter
  Real _wi;

  /// Derivative of the double well function \f$ \frac d{d\eta} g(\eta) \f$
  const MaterialProperty<Real> & _prop_dgi;

  /// Second derivative of the double well function \f$ \frac {d^2}{d\eta^2} g(\eta) \f$
  const MaterialProperty<Real> & _prop_d2gi;
};
