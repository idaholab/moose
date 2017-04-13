/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef KKSMULTIPHASECONCENTRATION_H
#define KKSMULTIPHASECONCENTRATION_H

#include "Kernel.h"
#include "DerivativeMaterialInterface.h"

// Forward Declarations
class KKSMultiPhaseConcentration;

template <>
InputParameters validParams<KKSMultiPhaseConcentration>();

/**
 * Enforce sum of phase concentrations to be the real concentration.
 *
 * \f$ c = h_1(\eta_1,\eta_2,\eta_3,...) c_1 + h_2(\eta_1,\eta_2,\eta_3,...) c_2
 * + h_3(\eta_1,\eta_2,\eta_3,..) c_3 + ... \f$
 *
 * The non-linear variable for this Kernel is one of  the concentrations \f$ c_i \f$, while
 * \f$ c_j \neq c_i \f$ and \f$ c \f$ are supplied as coupled variables.
 * The other phase concentrations are set as non-linear variables using multiple
 * KKSPhaseChemicalPotential kernels.
 *
 * \see KKSPhaseChemicalPotential
 */
class KKSMultiPhaseConcentration : public DerivativeMaterialInterface<Kernel>
{
public:
  KKSMultiPhaseConcentration(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

private:
  const unsigned int _num_j;
  std::vector<const VariableValue *> _cjs;
  std::vector<unsigned int> _cjs_var;
  /// Position of the nonlinear variable in the list of cj's
  int _k;

  const VariableValue & _c;
  unsigned int _c_var;

  /// Switching functions for each phase \f$ h_j \f$
  std::vector<MaterialPropertyName> _hj_names;
  std::vector<const MaterialProperty<Real> *> _prop_hj;

  /// Order parameters for each phase \f$ \eta_j \f$
  std::vector<VariableName> _eta_names;
  std::vector<unsigned int> _eta_vars;

  /// Derivative of the switching function \f$ \frac d{d\eta} h(\eta) \f$
  std::vector<std::vector<const MaterialProperty<Real> *>> _prop_dhjdetai;
};

#endif // KKSMULTIPHASECONCENTRATION_H
