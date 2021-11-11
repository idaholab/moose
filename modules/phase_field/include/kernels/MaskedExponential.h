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
 * This kernel implements a term in the variationally-derived equivalent form of Poisson's equation
 * for the electrochemical grand potential sintering model with dilute solution energetics.
 * It acts on the electric potential.
 */
class MaskedExponential : public DerivativeMaterialInterface<JvarMapKernelInterface<Kernel>>
{
public:
  static InputParameters validParams();

  MaskedExponential(const InputParameters & parameters);
  virtual void initialSetup();

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  /// Chemical potential
  unsigned int _w_var;
  const VariableValue & _w;

  /// Temperature
  std::string _temp_name;
  unsigned int _temp_var;
  const VariableValue & _temp;

  /// Mask
  const MaterialProperty<Real> & _mask;
  std::vector<const MaterialProperty<Real> *> _prop_dmaskdarg;

  /// Equilibrium defect concentration
  const MaterialProperty<Real> & _n_eq;
  const MaterialProperty<Real> & _prop_dn_eqdT;
  std::vector<const MaterialProperty<Real> *> _prop_dn_eqdarg;

  /// Species charge
  const int _z;

  /// Boltzmann constant
  const Real _kB;

  /// Electron charge
  const Real _e;
};
