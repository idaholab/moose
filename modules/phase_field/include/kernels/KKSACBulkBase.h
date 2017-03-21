/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef KKSACBULKBASE_H
#define KKSACBULKBASE_H

#include "ACBulk.h"

// Forward Declarations
class KKSACBulkBase;

template <>
InputParameters validParams<KKSACBulkBase>();

/**
 * ACBulk child class that takes all the necessary data from a
 * KKSBaseMaterial and sets up the Allen-Cahn bulk term.
 *
 * The non-linear variable for this Kernel is the order parameter 'eta'.
 */
class KKSACBulkBase : public ACBulk<Real>
{
public:
  KKSACBulkBase(const InputParameters & parameters);

  virtual void initialSetup();

protected:
  /// Number of coupled variables
  unsigned int _nvar;

  /// name of the order parameter (needed to retrieve the derivative material properties)
  VariableName _eta_name;

  /// Derivatives of \f$ F_a \f$ with respect to all coupled variables
  std::vector<const MaterialProperty<Real> *> _derivatives_Fa;

  /// Derivatives of \f$ F_b \f$ with respect to all coupled variables
  std::vector<const MaterialProperty<Real> *> _derivatives_Fb;

  /// Value of the free energy function \f$ F_a \f$
  const MaterialProperty<Real> & _prop_Fa;

  /// Value of the free energy function \f$ F_b \f$
  const MaterialProperty<Real> & _prop_Fb;

  /// Derivative of the free energy function \f$ \frac d{d\eta} F_a \f$
  const MaterialProperty<Real> & _prop_dFa;

  /// Derivative of the free energy function \f$ \frac d{d\eta} F_b \f$
  const MaterialProperty<Real> & _prop_dFb;

  /// Derivative of the switching function \f$ \frac d{d\eta} h(\eta) \f$
  const MaterialProperty<Real> & _prop_dh;

  /// Second derivative of the switching function \f$ \frac {d^2}{d\eta^2} h(\eta) \f$
  const MaterialProperty<Real> & _prop_d2h;

  /// Gradients for all coupled variables
  std::vector<const VariableGradient *> _grad_args;
};

#endif // KKSACBULKBASE_H
