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
 * KKSACBulkBase child class for the phase concentration term
 * \f$ - \sum_j \frac{dF_1}{dc_1} \frac{dh_j}{d\eta_i} (c_j) \f$
 * in the the Allen-Cahn bulk residual.
 *
 * The non-linear variable for this Kernel is the order parameter 'eta_i'.
 */
class NestKKSMultiACBulkC : public KKSMultiACBulkBase {
public:
  static InputParameters validParams();

  NestKKSMultiACBulkC(const InputParameters &parameters);

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
  std::vector<std::vector<MaterialPropertyName>> _ci_name_matrix;
  std::vector<std::vector<const MaterialProperty<Real> *>> _prop_ci;
  std::vector<std::vector<std::vector<const MaterialProperty<Real> *>>>
      _dcidetaj;
  std::vector<std::vector<std::vector<const MaterialProperty<Real> *>>> _dcidb;

  std::vector<std::vector<const MaterialProperty<Real> *>> _prop_d2hjdetaidetap;
  std::vector<const MaterialProperty<Real> *> _dF1dc1;
  std::vector<std::vector<const MaterialProperty<Real> *>> _d2F1dc1db1;
  std::vector<std::vector<const MaterialProperty<Real> *>> _d2F1dc1darg;
};
