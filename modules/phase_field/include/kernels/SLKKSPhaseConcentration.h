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

/**
 * Enforce sum of phase concentrations to be the real concentration.
 *
 * \f$ c=h(\eta)c_a+\left(1-h(\eta)\right)c_b\f$
 *
 * The non-linear variable for this Kernel is the concentration \f$ c_b \f$, while
 * \f$ c_a \f$ and \f$ c \f$ are supplied as coupled variables.
 * (compare this to KKSPhaseChemicalPotential, where the non-linear variable is
 * the other phase concentration \f$ c_a \f$!)
 * D. Schwen et al. https://doi.org/10.1016/j.commatsci.2021.110466
 *
 * \see KKSPhaseChemicalPotential
 * \see KKSHEtaPolyMaterial
 */
class SLKKSPhaseConcentration : public DerivativeMaterialInterface<JvarMapKernelInterface<Kernel>>
{
public:
  static InputParameters validParams();

  SLKKSPhaseConcentration(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  ///@{ sublattice A variables
  unsigned int _nca;
  std::vector<const VariableValue *> _ca;
  std::vector<Real> _a_ca;
  const JvarMap & _ca_map;
  ///@}

  ///@{ sublattice B variables
  unsigned int _ncb;
  std::vector<const VariableValue *> _cb;
  std::vector<Real> _a_cb;
  const JvarMap & _cb_map;
  ///@}

  /// sublattice fraction for the sublattice B concentration represented by the kernel variable
  Real _a_u;

  ///@{ global concentration variable
  const VariableValue & _c;
  unsigned int _c_var;
  ///@}

  ///@{ phase order parameter
  const VariableValue & _eta;
  unsigned int _eta_var;
  ///@}

  /// Switching function \f$ h(\eta) \f$
  const MaterialProperty<Real> & _prop_h;

  /// Derivative of the switching function \f$ \frac d{d\eta} h(\eta) \f$
  const MaterialProperty<Real> & _prop_dh;

private:
  /// update the _casum and _cbsum members
  void computeSums();

  ///@{ updated by computeSums
  Real _casum;
  Real _cbsum;
  ///@}
};
