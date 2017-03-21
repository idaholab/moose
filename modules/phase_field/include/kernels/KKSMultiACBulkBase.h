/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef KKSMULTIACBULKBASE_H
#define KKSMULTIACBULKBASE_H

#include "ACBulk.h"

// Forward Declarations
class KKSMultiACBulkBase;

template <>
InputParameters validParams<KKSMultiACBulkBase>();

/**
 * ACBulk child class that sets up necessary variables and materials for
 * calculation of residual contribution \f$ \frac{\partial f}{\partial \eta_i} \f$
 * by child classes KKSMultiACBulkF and KKSMultiACBulkC.
 *
 * The non-linear variable for this Kernel is the order parameter \f$ \eta_i \f$.
 */
class KKSMultiACBulkBase : public ACBulk<Real>
{
public:
  KKSMultiACBulkBase(const InputParameters & parameters);

  virtual void initialSetup();

protected:
  /// Number of coupled variables
  unsigned int _nvar;

  /// name of order parameter that derivatives are taken wrt (needed to retrieve the derivative material properties)
  VariableName _etai_name;

  /// index of order parameter that derivatives are taken wrt
  unsigned int _etai_var;

  /// Names of free energy functions for each phase \f$ F_j \f$
  std::vector<MaterialPropertyName> _Fj_names;
  unsigned int _num_j;

  /// Values of the free energy functions for each phase \f$ F_j \f$
  std::vector<const MaterialProperty<Real> *> _prop_Fj;

  /// Derivatives of the free energy functions (needed for off-diagonal Jacobians)
  std::vector<std::vector<const MaterialProperty<Real> *>> _prop_dFjdarg;

  /// switching function names
  std::vector<MaterialPropertyName> _hj_names;

  /// Values of the switching functions for each phase \f$ h_j \f$
  std::vector<const MaterialProperty<Real> *> _prop_hj;

  /// Derivatives of the switching functions wrt the order parameter for this kernel
  std::vector<const MaterialProperty<Real> *> _prop_dhjdetai;

  /// Second derivatives of the switching functions wrt the order parameter for this kernel
  std::vector<const MaterialProperty<Real> *> _prop_d2hjdetai2;

  /// Second derivatives of the switching functions (needed for off-diagonal Jacobians)
  std::vector<std::vector<const MaterialProperty<Real> *>> _prop_d2hjdetaidarg;
};

#endif // KKSMULTIACBULKBASE_H
