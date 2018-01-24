/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef KKSCHBULK_H
#define KKSCHBULK_H

#include "CHBulk.h"

// Forward Declarations
class KKSCHBulk;

template <>
InputParameters validParams<KKSCHBulk>();

/**
 * CHBulk child class that takes all the necessary data from a
 * KKSBaseMaterial.
 * We calculate \f$ \nabla\frac{\partial F_a}{\partial c_a} \f$.
 * This takes advantage of the KKS identity
 *
 * \f$ dF/dc = dF_a/dc_a (= dF_b/dc_b) \f$
 *
 * The non-linear variable for this Kernel is the concentration 'c'.
 * The user picks one phase free energy \f$ F_a \f$ (f_base) and its corresponding
 * phase concentration \f$ c_a \f$
 */
class KKSCHBulk : public CHBulk<Real>
{
public:
  KKSCHBulk(const InputParameters & parameters);

protected:
  virtual RealGradient computeGradDFDCons(PFFunctionType type);
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

private:
  /// Number of coupled variables
  unsigned int _nvar;

  ///@{
  /// Phase concnetration variables
  unsigned int _ca_var;
  const VariableName _ca_name;
  unsigned int _cb_var;
  const VariableName _cb_name;
  ///@}

  /// Derivatives of \f$ dFa/dca \f$ with respect to all coupled variables
  std::vector<const MaterialProperty<Real> *> _second_derivatives;

  /// Second derivatives of dFa/dca with respect to all coupled variables
  std::vector<std::vector<const MaterialProperty<Real> *>> _third_derivatives;

  /// Derivatives of \f$ d^2Fa/dca^2 \f$ with respect to all coupled variables
  std::vector<const MaterialProperty<Real> *> _third_derivatives_ca;

  /// Gradients for all coupled variables
  std::vector<const VariableGradient *> _grad_args;

  /// h(eta) material property
  const MaterialProperty<Real> & _prop_h;

  /// Second derivative \f$ d^2Fa/dca^2 \f$
  const MaterialProperty<Real> & _second_derivative_Fa;

  /// Second derivative \f$ d^2Fb/dcb^2 \f$
  const MaterialProperty<Real> & _second_derivative_Fb;
};

#endif // KKSCHBULK_H
