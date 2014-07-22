#ifndef KKSPHASECHEMICALPOTENTIAL_H
#define KKSPHASECHEMICALPOTENTIAL_H

#include "Kernel.h"
#include "JvarMapInterface.h"
#include "DerivativeMaterialInterface.h"

//Forward Declarations
class KKSPhaseChemicalPotential;

template<>
InputParameters validParams<KKSPhaseChemicalPotential>();

/**
 * Enforce the equality of the chemical potentials in the two phases.
 * Eq. (21) in the original KKS paper.
 *
 * \f$ dF_a/dc_a = dF_b/dc_b \f$
 *
 * We need to supply two free energy functions (i.e. KKSBaseMaterial) by giving
 * two "base names" ('Fa', 'Fb'). We supply concentration \f$ c_a \f$ as the non-linear
 * variable and \f$ c_b \f$ as a coupled variable
 * (compare this to KKSPhaseConcentration, where the non-linear variable is
 * the other phase concentration \f$ c_b \f$!)
 *
 * \see KKSPhaseConcentration
 */
class KKSPhaseChemicalPotential : public DerivativeMaterialInterface<
                                         JvarMapInterface<
                                         Kernel
                                         > >
{
public:
  KKSPhaseChemicalPotential(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

private:
  /// coupled variable for cb
  unsigned int _cb_var;
  std::string _cb_name;


  /// free energy function material property base name
  std::string _Fa_name;
  /// free energy function material property base name
  std::string _Fb_name;

  /// material properties we need to access
  MaterialProperty<Real> & _dfadca;
  MaterialProperty<Real> & _dfbdcb;
  MaterialProperty<Real> & _d2fadca2;
  MaterialProperty<Real> & _d2fbdcbca;

  std::vector<MaterialProperty<Real>* > _off_diag_a;
  std::vector<MaterialProperty<Real>* > _off_diag_b;
};

#endif //KKSPHASECHEMICALPOTENTIAL_H
