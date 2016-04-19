/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef KKSMULTIACBULKC_H
#define KKSMULTIACBULKC_H

#include "KKSMultiACBulkBase.h"

//Forward Declarations
class KKSMultiACBulkC;

template<>
InputParameters validParams<KKSMultiACBulkC>();

/**
 * KKSACBulkBase child class for the phase concentration difference term
 * \f$ \frac{dh}{d\eta}\frac{dF_a}{dc_a}(c_a-c_b) \f$
 * in the the Allen-Cahn bulk residual.
 *
 * The non-linear variable for this Kernel is the order parameter 'eta_i'.
 */
class KKSMultiACBulkC : public KKSMultiACBulkBase
{
public:
  KKSMultiACBulkC(const InputParameters & parameters);

protected:
  virtual Real computeDFDOP(PFFunctionType type);
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  const unsigned int _ncj;
  /// Names of phase concentration variables
  //std::vector<MaterialPropertyName> _cj_names;
  MaterialPropertyName _c1_name;
  std::vector<const VariableValue *> _cjs;
  std::vector<unsigned int> _cjs_var;

  // /// phase b concentration
  // std::string _cb_name;
  // unsigned int _cb_var;
  // const VariableValue & _cb;
  //
  // /// Value of the switching function \f$ h(\eta) \f$
  // const MaterialProperty<Real> & _prop_h;
  //
  /// Derivative of the free energy function \f$ \frac d{dc_1} F_1 \f$
  const MaterialProperty<Real> & _prop_dF1dc1;
  /// Second derivative of the free energy function \f$ \frac {d^2}{dc_1^2} F_1 \f$
  const MaterialProperty<Real> & _prop_d2F1dc12;
  /// Mixed partial derivatives of the free energy function wrt c1 and
  /// any other coupled variables \f$ \frac {d^2}{dc_1 dv} F_1 \f$
  std::vector<const MaterialProperty<Real>* > _prop_d2F1dc1darg;
};

#endif //KKSMULTIACBULKC_H
