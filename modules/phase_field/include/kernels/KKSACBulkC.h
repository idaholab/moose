/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef KKSACBULKC_H
#define KKSACBULKC_H

#include "KKSACBulkBase.h"

// Forward Declarations
class KKSACBulkC;

template <>
InputParameters validParams<KKSACBulkC>();

/**
 * KKSACBulkBase child class for the phase concentration difference term
 * \f$ \frac{dh}{d\eta}\frac{dF_a}{dc_a}(c_a-c_b) \f$
 * in the the Allen-Cahn bulk residual.
 *
 * The non-linear variable for this Kernel is the order parameter 'eta'.
 */
class KKSACBulkC : public KKSACBulkBase
{
public:
  KKSACBulkC(const InputParameters & parameters);

protected:
  virtual Real computeDFDOP(PFFunctionType type);
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  /// phase a concentration
  std::string _ca_name;
  unsigned int _ca_var;
  const VariableValue & _ca;

  /// phase b concentration
  std::string _cb_name;
  unsigned int _cb_var;
  const VariableValue & _cb;

  /// Value of the switching function \f$ h(\eta) \f$
  const MaterialProperty<Real> & _prop_h;

  /// Derivative of the free energy function \f$ \frac d{dc_a} F_a \f$
  const MaterialProperty<Real> & _prop_dFadca;

  /// Second derivative of the free energy function \f$ \frac {d^2}{dc_a^2} F_a \f$
  const MaterialProperty<Real> & _prop_d2Fadca2;

  /// Second derivative of the free energy function \f$ \frac {d^2}{dc_b^2} F_b \f$
  const MaterialProperty<Real> & _prop_d2Fbdcb2;

  /// Mixed partial derivatives of the free energy function wrt ca and
  /// any other coupled variables \f$ \frac {d^2}{dc_a dq} F_a \f$
  std::vector<const MaterialProperty<Real> *> _prop_d2Fadcadarg;
};

#endif // KKSACBULKC_H
