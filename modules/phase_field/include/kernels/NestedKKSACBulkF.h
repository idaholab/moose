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

/**
 * KKSACBulkBase child class for the free energy difference term
 * \f$ -\frac{dh}{d\eta}(F_a-F_b)+w\frac{dg}{d\eta} \f$
 * in the the Allen-Cahn bulk residual.
 */
class NestedKKSACBulkF : public KKSACBulkBase
{
public:
  static InputParameters validParams();

  NestedKKSACBulkF(const InputParameters & parameters);

protected:
  virtual Real computeDFDOP(PFFunctionType type);
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  const JvarMap & _c_map;

  /// Number of global concentrations
  const unsigned int _num_c;

  /// Global concentrations
  const std::vector<VariableName> _c_names;

  /// Phase concentrations
  const std::vector<MaterialPropertyName> _ci_names;

  /// Free energy of phase a
  const MaterialPropertyName _Fa_name;

  /// Derivative of the free energy function \f$ \frac d{dc_a} F_a \f$
  std::vector<const MaterialProperty<Real> *> _dFadca;

  /// Free energy of phase b
  const MaterialPropertyName _Fb_name;

  /// Derivative of the free energy function \f$ \frac d{dc_b} F_b \f$
  std::vector<const MaterialProperty<Real> *> _dFbdcb;

  /// Derivative of barrier function g
  const MaterialProperty<Real> & _prop_dg;

  /// Second derivative of barrier function g
  const MaterialProperty<Real> & _prop_d2g;

  /// Double well height parameter
  const Real _w;

  /// Free energy properties
  std::vector<const MaterialProperty<Real> *> _prop_Fi;

  /// Derivative of phase concentration wrt eta \f$ \frac d{d{eta}} c_i \f$
  std::vector<std::vector<const MaterialProperty<Real> *>> _dcideta;

  /// Derivative of phase concentration wrt global concentration \f$ \frac d{b} c_i \f$
  std::vector<std::vector<std::vector<const MaterialProperty<Real> *>>> _dcidb;

  /// Partial derivative of the free energy function Fa wrt coupled variables \f$ \frac d{dq} F_a \f$
  std::vector<const MaterialProperty<Real> *> _dFadarg;

  /// Partial derivative of the free energy function Fb wrt coupled variables \f$ \frac d{dq} F_b \f$
  std::vector<const MaterialProperty<Real> *> _dFbdarg;
};
