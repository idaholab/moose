//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SLKKSMultiPhaseBase.h"

/**
 * SLKKSMultiPhaseBase child class for the phase concentration term
 * \f$ - \sum_j \frac{dF_1}{dc_1} \frac{dh_j}{d\eta_i} (c_j) \f$
 * in the the Allen-Cahn bulk residual.
 * D. Schwen et al. https://doi.org/10.1016/j.commatsci.2021.110466
 *
 * The non-linear variable for this Kernel is the order parameter 'eta_i'.
 */
class SLKKSMultiACBulkC : public SLKKSMultiPhaseBase
{
public:
  static InputParameters validParams();

  SLKKSMultiACBulkC(const InputParameters & parameters);

protected:
  virtual Real precomputeQpResidual();
  virtual Real precomputeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  /// name of the coupled concentration variable c
  const VariableName _c_name;

  /// is eta_i supplied (then we assume the kernel operates on the Lagrange var)
  const bool _lagrange;

  /// name of order parameter that derivatives are taken w.r.t.
  VariableName _etai_name;

  /// index of order parameter that derivatives are taken w.r.t.
  unsigned int _etai_var;

  /// names of all sublattice concentrations
  std::vector<VariableName> _cs_names;

  /// Derivative of the free energy function w.r.t. c
  const MaterialProperty<Real> & _prop_dFdc;

  /// Second derivatives of F w.r.t. c and all cs
  std::vector<const MaterialProperty<Real> *> _prop_d2Fdcdcs;

  /// first derivatives of all h w.r.t. to etai
  std::vector<const MaterialProperty<Real> *> _prop_dhdni;

  /// first derivatives of all h w.r.t. to the kernel variable and other etas
  std::vector<std::vector<const MaterialProperty<Real> *>> _prop_d2hdnidn;

  /// Position of the c variable in the cs list
  int _l_cs;

  /// Position of the eta_i variable in the eta list
  int _l_etai;

  /// Mobility
  const MaterialProperty<Real> & _mob;
};
