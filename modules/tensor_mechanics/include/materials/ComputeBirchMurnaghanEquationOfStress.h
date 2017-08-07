/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef COMPUTEBIRCHMURNAGHANEQUATIONOFSTRESS_H
#define COMPUTEBIRCHMURNAGHANEQUATIONOFSTRESS_H

#include "ComputeStressBase.h"

/**
 * ComputeBirchMurnaghanEquationOfStress adds a volumetric extra stress including
 * a Birch-Murnaghan equation of state
 * and bulk viscosity damping for shock propagation
 * that is substituted to the volumetric stress
 * calculated by the constitutive model.
 * Austin et al. JOURNAL OF APPLIED PHYSICS 117, 185902 (2015)
 * Maheo et al. Mechanics Research Communications 38 (2011) 81 88
 */
class ComputeBirchMurnaghanEquationOfStress;

template <>
InputParameters validParams<ComputeBirchMurnaghanEquationOfStress>();

class ComputeBirchMurnaghanEquationOfStress : public ComputeStressBase
{
public:
  ComputeBirchMurnaghanEquationOfStress(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

  // exponent in Birch-Murnaghan equation of state
  const Real _n_Murnaghan;

  // reference bulk modulus in Birch-Murnaghan equation of state
  const Real _Bulk_Modulus_Ref;

  // Von Neumann damping coefficient
  const Real _C0;

  // Landshoff damping coefficient
  const Real _C1;

  const MaterialProperty<RankTwoTensor> & _deformation_gradient;
  const MaterialProperty<RankTwoTensor> & _deformation_gradient_old;
};

#endif // COMPUTEBIRCHMURNAGHANEQUATIONOFSTRESS_H
