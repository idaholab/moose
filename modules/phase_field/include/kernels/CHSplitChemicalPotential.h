/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef CHSPLITCHEMICALPOTENTIAL_H
#define CHSPLITCHEMICALPOTENTIAL_H

#include "Kernel.h"
#include "DerivativeMaterialInterface.h"

class CHSplitChemicalPotential;

template <>
InputParameters validParams<CHSplitChemicalPotential>();

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

#endif
