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
#include "DerivativeMaterialInterface.h"

// Forward Declarations

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
  static InputParameters validParams();

  KKSPhaseConcentration(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

private:
  const VariableValue & _ca;
  unsigned int _ca_var;

  const VariableValue & _c;
  unsigned int _c_var;

  const VariableValue & _eta;
  unsigned int _eta_var;

  /// Switching function \f$ h(\eta) \f$
  const MaterialProperty<Real> & _prop_h;

  /// Derivative of the switching function \f$ \frac d{d\eta} h(\eta) \f$
  const MaterialProperty<Real> & _prop_dh;
};
