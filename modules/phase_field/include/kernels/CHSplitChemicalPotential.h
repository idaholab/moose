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

/**
 * Solves chemical potential in a weak sense (mu-mu_prop=0)
 * Can be coupled to Cahn-Hilliard equation to solve species diffusion
 * Allows spatial derivative of chemical potential when coupled to material state such as stress,
 *etc.
 * Can be used to model species diffusion mediated creep
 **/
class CHSplitChemicalPotential : public DerivativeMaterialInterface<Kernel>
{
public:
  static InputParameters validParams();

  CHSplitChemicalPotential(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  ///Chemical potential property evaluated at material points
  MaterialPropertyName _mu_prop_name;

  const MaterialProperty<Real> & _chemical_potential;
  const MaterialProperty<Real> & _dchemical_potential_dc;
  const unsigned int _c_var;
};
