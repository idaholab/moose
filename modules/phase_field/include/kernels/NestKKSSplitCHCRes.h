//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "DerivativeMaterialInterface.h"
#include "JvarMapInterface.h"
#include "Kernel.h"

// Forward Declarations

/**
 * SplitCHBulk child class that takes all the necessary data from a
 * KKSBaseMaterial.
 * We calculate \f$ \frac{\partial F_a}{\partial c_a} \f$.
 * This takes advantage of the KKS identity
 *
 * \f$ dF/dc = dF_a/dc_a (= dF_b/dc_b) \f$
 *
 * The non-linear variable for this Kernel is the concentration 'c'.
 * The user picks one phase free energy \f$ F_a \f$ (f_base) and its
 * corresponding phase concentration \f$ c_a \f$
 */

class NestKKSSplitCHCRes : public DerivativeMaterialInterface<JvarMapKernelInterface<Kernel>>
{
public:
  static InputParameters validParams();

  NestKKSSplitCHCRes(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

  /// Phase parameters
  const std::vector<VariableName> _eta_names;

  /// Number of phase parameters
  const unsigned int _num_j;
  const JvarMap & _eta_map;

  /// Global concentrations
  const std::vector<VariableName> _c_names;

  /// Number of global concentrations
  const unsigned int _num_c;
  const JvarMap & _c_map;
  int _o;
  /// Chemical potential
  const unsigned int _w_var;
  const VariableValue & _w;

  /// Phase concentration of the first phase in _eta_names
  const std::vector<MaterialPropertyName> _c1_names;

  /// Free energy
  const MaterialPropertyName _F1_name;

  /// Derivative of the free energy function \f$ \frac d{dc_1} F_1 \f$
  std::vector<const MaterialProperty<Real> *> _dF1dc1;

  /// Second derivative of the free energy function \f$ \frac {d^2}{dc_1 db_1}
  /// F_1 \f$
  std::vector<const MaterialProperty<Real> *> _d2F1dc1db1;

  /// Derivative of the phase concentrations wrt global concentrations \f$ \frac
  /// d{db} c_1 \f$
  std::vector<std::vector<const MaterialProperty<Real> *>> _dc1db;

  /// Derivative of the phase concentrations wrt phase parameter \f$ \frac
  /// d{d{eta}} c_1 \f$
  std::vector<std::vector<const MaterialProperty<Real> *>> _dc1detaj;

  /// Second derivative of the free energy function wrt phase concentration and
  /// a coupled variable
  std::vector<const MaterialProperty<Real> *> _d2F1dc1darg;
};
