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
 * KKSACBulkBase child class for the phase concentration difference term
 * \f$ \frac{dh}{d\eta}\frac{\partial F_a}{\partial c_a}(c_a-c_b) \f$
 * in the the Allen-Cahn bulk residual.
 */
class NestedKKSACBulkC : public KKSACBulkBase
{
public:
  static InputParameters validParams();

  NestedKKSACBulkC(const InputParameters & parameters);

protected:
  virtual Real computeDFDOP(PFFunctionType type);
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  /// Global concentrations
  std::vector<VariableName> _c_names;
  const JvarMap & _c_map;

  /// Number of global concentrations
  const unsigned int _num_c;

  /// Phase concentrations
  const std::vector<MaterialPropertyName> _ci_names;

  /// Phase concentration properties
  std::vector<std::vector<const MaterialProperty<Real> *>> _prop_ci;

  /// Derivative of phase concentrations wrt eta \f$ \frac d{d{eta}} c_i \f$
  std::vector<std::vector<const MaterialProperty<Real> *>> _dcideta;

  /// Derivative of phase concentrations wrt global concentrations \f$ \frac d{db} c_i \f$
  std::vector<std::vector<std::vector<const MaterialProperty<Real> *>>> _dcidb;

  /// Free energy of phase a
  const MaterialPropertyName _Fa_name;

  /// Derivative of the free energy function \f$ \frac d{dc_a} F_a \f$
  std::vector<const MaterialProperty<Real> *> _dFadca;

  /// Second derivative of the free energy function \f$ \frac {d^2}{dc_a db_a} F_a \f$
  std::vector<std::vector<const MaterialProperty<Real> *>> _d2Fadcadba;

  /// Mixed partial derivatives of the free energy function wrt c and any other coupled variables \f$
  /// \frac {d^2}{dc_a dq} F_a \f$
  std::vector<std::vector<const MaterialProperty<Real> *>> _d2Fadcadarg;
};
