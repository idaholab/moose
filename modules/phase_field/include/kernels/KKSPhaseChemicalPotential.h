//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Kernel.h"
#include "JvarMapInterface.h"
#include "DerivativeMaterialInterface.h"

// Forward Declarations

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
class KKSPhaseChemicalPotential : public DerivativeMaterialInterface<JvarMapKernelInterface<Kernel>>
{
public:
  static InputParameters validParams();

  KKSPhaseChemicalPotential(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);
  virtual void initialSetup();

private:
  /// coupled variable for cb
  unsigned int _cb_var;
  VariableName _cb_name;

  /// material properties we need to access
  const MaterialProperty<Real> & _dfadca;
  const MaterialProperty<Real> & _dfbdcb;
  const MaterialProperty<Real> & _d2fadca2;
  const MaterialProperty<Real> & _d2fbdcbca;

  std::vector<const MaterialProperty<Real> *> _d2fadcadarg;
  std::vector<const MaterialProperty<Real> *> _d2fbdcbdarg;

  ///@{ site fractions
  const Real _ka;
  const Real _kb;
  ///@}
};
