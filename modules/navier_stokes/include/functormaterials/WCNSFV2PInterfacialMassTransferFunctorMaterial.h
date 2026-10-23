//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FunctorMaterial.h"

/**
 * Computes the interfacial mass transfer rate of the two-phase mixture model from the transported
 * interfacial area concentration, following the interfacial energy jump condition, the heat
 * reaching the interface divided by the latent heat,
 *
 * \f[
 *   \Gamma = \frac{\chi_p h_i \left(T - T_{sat}\right)}{h_{fg}} ,
 * \f]
 *
 * positive generating the dispersed phase, with \f$ h_i \f$ the bubbly-flow interfacial Nusselt
 * number of RELAP5/MOD3, NUREG/CR-5535 Volume 4 Section 4.1.1.1.1. The driving potential is the
 * departure of the mixture temperature from saturation, there being one energy equation. The
 * substitution of the transported area for the algebraic area of that reference, and the limits it
 * carries, are set out in the documentation page.
 */
class WCNSFV2PInterfacialMassTransferFunctorMaterial : public FunctorMaterial
{
public:
  static InputParameters validParams();

  WCNSFV2PInterfacialMassTransferFunctorMaterial(const InputParameters & parameters);

protected:
  /// The dimension of the simulation
  const unsigned int _dim;

  /// Shape factor of the averaged particle size, 6 for spheres
  const Real _shape_factor;

  /// Interfacial area concentration, the transported area this closure is built on
  const Moose::Functor<Real> & _interfacial_area;

  /// Volume fraction of the dispersed phase
  const Moose::Functor<Real> & _fd;

  /// Temperature of the mixture, shared by the phases
  const Moose::Functor<Real> & _temperature;

  /// Saturation temperature of the transition the transfer represents
  const Moose::Functor<Real> & _T_saturation;

  /// Latent heat of that transition, per unit mass
  const Moose::Functor<Real> & _latent_heat;

  /// Continuous phase properties, which carry the interfacial heat transfer
  const Moose::Functor<Real> & _rho_c;
  const Moose::Functor<Real> & _mu_c;
  const Moose::Functor<Real> & _k_c;

  /// Components of the slip velocity, which set the particle Reynolds number
  const Moose::Functor<Real> & _u_slip;
  const Moose::Functor<Real> * const _v_slip;
  const Moose::Functor<Real> * const _w_slip;
};
