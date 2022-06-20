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
 * \f$ \sum_j \frac{\partial h_j}{\partial \eta_i} F_j + w_i \frac{dg}{d\eta_i}
 * \f$ in the the Allen-Cahn bulk residual.
 *
 * The non-linear variable for this Kernel is the order parameter \f$ eta_i \f$.
 */
class NestKKSMultiACBulkF : public KKSMultiACBulkBase {
public:
  static InputParameters validParams();

  NestKKSMultiACBulkF(const InputParameters &parameters);

protected:
  virtual Real computeDFDOP(PFFunctionType type);
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  std::vector<VariableName> _c_names;
  const JvarMap &_c_map;
  unsigned int _num_c;
  std::vector<VariableName> _eta_names;
  const JvarMap &_eta_map;
  /// Position of the nonlinear variable in the list of cj's
  int _k;
  std::vector<MaterialPropertyName> _ci_names;

  std::vector<std::vector<std::vector<const MaterialProperty<Real> *>>>
      _dcidetaj;
  std::vector<std::vector<std::vector<const MaterialProperty<Real> *>>> _dcidb;

  /// double well height parameter
  Real _wi;
  MaterialPropertyName _gi_name;
  const MaterialProperty<Real> &_dgi;
  const MaterialProperty<Real> &_d2gi;
  std::vector<std::vector<const MaterialProperty<Real> *>> _d2hjdetaidetap;
  std::vector<const MaterialProperty<Real> *> _dF1dc1;
  std::vector<std::vector<const MaterialProperty<Real> *>> _dFidarg;
};
