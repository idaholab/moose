#ifndef KKSACBULKBASE_H
#define KKSACBULKBASE_H

#include "ACBulk.h"
#include "JvarMapInterface.h"
#include "DerivativeMaterialInterface.h"

//Forward Declarations
class KKSACBulkBase;

template<>
InputParameters validParams<KKSACBulkBase>();

/**
 * ACBulk child class that takes all the necessary data from a
 * KKSBaseMaterial and sets up the Allen-Cahn bulk term.
 *
 * The non-linear variable for this Kernel is the order parameter 'eta'.
 */
class KKSACBulkBase : public DerivativeMaterialInterface<
                         JvarMapInterface<
                         ACBulk
                         > >
{
public:
  KKSACBulkBase(const std::string & name, InputParameters parameters);

protected:
  /// Number of coupled variables
  unsigned int _nvar;

  /// name of the order parameter (needed to retrieve the derivative material properties)
  std::string _eta_name;

  /// phase a free energy function material property base names
  std::string _Fa_name;

  /// phase b free energy function material property base names
  std::string _Fb_name;

  /// switching function material property base names
  std::string _h_name;

  /// Derivatives of \f$ F_a \f$ with respect to all coupled variables
  std::vector<MaterialProperty<Real> *> _derivatives_Fa;

  /// Derivatives of \f$ F_b \f$ with respect to all coupled variables
  std::vector<MaterialProperty<Real> *> _derivatives_Fb;

  /// Value of the free energy function \f$ F_a \f$
  MaterialProperty<Real> & _prop_Fa;

  /// Value of the free energy function \f$ F_b \f$
  MaterialProperty<Real> & _prop_Fb;

  /// Derivative of the free energy function \f$ \frac d{d\eta} F_a \f$
  MaterialProperty<Real> & _prop_dFa;

  /// Derivative of the free energy function \f$ \frac d{d\eta} F_b \f$
  MaterialProperty<Real> & _prop_dFb;

  /// Derivative of the switching function \f$ \frac d{d\eta} h(\eta) \f$
  MaterialProperty<Real> & _prop_dh;

  /// Second derivative of the switching function \f$ \frac {d^2}{d\eta^2} h(\eta) \f$
  MaterialProperty<Real> & _prop_d2h;

  /// Gradients for all coupled variables
  std::vector<VariableGradient*> _grad_args;
};

#endif //KKSACBULKBASE_H
