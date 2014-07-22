#ifndef KKSPHASECONCENTRATION_H
#define KKSPHASECONCENTRATION_H

#include "Kernel.h"
#include "DerivativeMaterialInterface.h"

//Forward Declarations
class KKSPhaseConcentration;

template<>
InputParameters validParams<KKSPhaseConcentration>();

/**
 * Enforce sum of phase concentrations to be the real concentration.
 *
 * \f$ c=h(\eta)c_a+\left(1-h(\eta)\right)c_b\f$
 *
 * The non-linear variable for this Kernel is the concentration \f$ c_b \f$, while
 * \f$ c_a \f$ and \f$ c \f$ are supplied as coupled variables.
 * (compare this to KKSPhaseChemicalPotential, where the non-linear variable is
 * the other phase concentration \f$ c_a \f$!)
 *
 * \see KKSPhaseChemicalPotential
 * \see KKSHEtaPolyMaterial
 */
class KKSPhaseConcentration : public DerivativeMaterialInterface<Kernel>
{
public:
  KKSPhaseConcentration(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

private:
  VariableValue & _ca;
  unsigned int _ca_var;

  VariableValue & _c;
  unsigned int _c_var;

  VariableValue & _eta;
  unsigned int _eta_var;

  /// switching function material property base names
  std::string _h_name;

  /// Switching function \f$ h(\eta) \f$
  MaterialProperty<Real> & _prop_h;

  /// Derivative of the switching function \f$ \frac d{d\eta} h(\eta) \f$
  MaterialProperty<Real> & _prop_dh;
};

#endif //KKSPHASECONCENTRATION_H
