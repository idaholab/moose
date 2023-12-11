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

/**
 * KKSMultiACBulkBase child class for the free energy term
 * \f$ \sum_j \frac{\partial h_j}{\partial \eta_i} F_j + w_i \frac{dg_i}{d\eta_i}
 * \f$ in the the Allen-Cahn bulk residual.
 *
 * The non-linear variable for this Kernel is the order parameter \f$ eta_i \f$.
 */
class NestedKKSMultiACBulkF : public KKSMultiACBulkBase
{
public:
  static InputParameters validParams();

  NestedKKSMultiACBulkF(const InputParameters & parameters);

protected:
  virtual Real computeDFDOP(PFFunctionType type);
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  ///@{ Global concentrations
  std::vector<VariableName> _c_names;
  const JvarMap & _c_map;
  ///@}

  /// Number of global concentrations
  unsigned int _num_c;

  ///@{ Phase parameters
  std::vector<VariableName> _eta_names;
  const JvarMap & _eta_map;
  ///@}

  /// Position of the nonlinear variable in the list of cj's
  int _k;

  /// Phase concentrations`
  std::vector<MaterialPropertyName> _ci_names;

  /// Derivative of phase concentrations wrt etaj \f$ \frac d{d{eta_j}} c_i \f$
  std::vector<std::vector<std::vector<const MaterialProperty<Real> *>>> _dcidetaj;

  /// Derivative of phase concentrations wrt global concentrations \f$ \frac d{db} c_i \f$
  std::vector<std::vector<std::vector<const MaterialProperty<Real> *>>> _dcidb;

  /// double well height parameter
  Real _wi;

  /// Barrier functions
  MaterialPropertyName _gi_name;

  /// Derivatives of barrier function
  const MaterialProperty<Real> & _dgi;

  /// Second derivative of barrier function
  const MaterialProperty<Real> & _d2gi;

  /// Second derivative of switching function \f$ \frac {d^2}{deta_i deta_p} h_j \f$
  std::vector<std::vector<const MaterialProperty<Real> *>> _d2hjdetaidetap;

  /// Derivative of the free energy function \f$ \frac d{dc_1} F_1 \f$
  std::vector<const MaterialProperty<Real> *> _dF1dc1;

  /// Derivative of the free energy function \f$ \frac d{dq} F_i \f$
  std::vector<std::vector<const MaterialProperty<Real> *>> _dFidarg;
};
