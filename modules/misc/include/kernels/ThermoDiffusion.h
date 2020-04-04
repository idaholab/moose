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
#include "Material.h"

/**
 * @brief Models thermo-diffusion (aka Soret effect, thermophoresis, etc.).
 * The mass flux J due to the thermal gradient is:
 *
 *   J_thermal = - ( D C Qstar / ( R T^2 ) ) * grad( T )
 *
 * where D is the mass diffusivity (same as for Fick's law), C is the concentration
 * Qstar is the heat of transport, R is the gas constant, and T is the temperature.
 * Note that that grad( T ) / T^2 == -grad( 1/T ), which is how the term
 * appears in Onsager's symmetry (this kernel works just as well for that form).
 *
 * Also note that coupled diffusion terms like ThermoDiffusion (and Nernst,
 * Ettingshausen, Dufour effects) can cause non-physical stability issues if a
 * regular diffusion term is not included. Thus, ThermoDiffusion should always
 * appear with a Fick's Law kernel such as CoefDiffusion.
 *
 * The only restriction on the units of C, Qstar, and R is that they should be
 * consistent with each other and with C and T (could be mass ppm, mols/m^3, etc.).
 *
 * This kernel applies to the concentration C, but the off-diagonal terms in the
 * Jacobian that are due to temperature dependence are also available.
 */
class ThermoDiffusion : public Kernel
{
public:
  static InputParameters validParams();

  ThermoDiffusion(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();

  virtual Real computeQpJacobian();

  /**
   * @brief Computes contribution from grad( T ) / T^2 term.
   */
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

private:
  RealGradient thermoDiffusionVelocity() const;

  const VariableValue & _temperature;
  const VariableGradient & _grad_temperature;
  const MaterialProperty<Real> & _mass_diffusivity;
  const MaterialProperty<Real> & _heat_of_transport;
  const Real _gas_constant;
  const unsigned int _temperature_index;
};
