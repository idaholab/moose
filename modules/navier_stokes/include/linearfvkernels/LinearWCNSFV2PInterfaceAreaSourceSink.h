//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "LinearFVElementalKernel.h"

/**
 * Sources and sinks of the one-group interfacial area concentration transport equation, for the
 * linear finite volume discretization.
 *
 * The transport equation, in conservative form,
 *
 * \f[
 *   \frac{\partial (\rho_g \chi_p)}{\partial t}
 *     + \nabla \cdot \left( \rho_g \mathbf{u}_g \chi_p \right)
 *   = \frac{1}{3}\frac{D\rho_g}{Dt}\chi_p
 *     + \frac{2}{3}\frac{\dot m_g}{\alpha_g}\chi_p
 *     + \rho_g \left( S_{RC} + S_{WE} + S_{TI} \right) ,
 * \f]
 *
 * with \f$ \chi_p \f$ the interfacial area concentration, \f$ \rho_g \f$ and
 * \f$ \alpha_g \f$ the density and volume fraction of the dispersed phase, \f$ \dot m_g \f$ the
 * mass transfer rate per unit mixture volume, and \f$ S_{RC} \f$, \f$ S_{WE} \f$,
 * \f$ S_{TI} \f$ the random collision and wake entrainment sinks and the turbulent impact source.
 * This object assembles everything but the time derivative and the advection.
 *
 * Two closure sets are offered, Hibiki and Ishii and Ishii and Kim, selected by 'model' and sharing
 * the averaged particle size \f$ d_b = \psi \alpha_g / \chi_p \f$. Their correlations are given
 * in the documentation page.
 */
class LinearWCNSFV2PInterfaceAreaSourceSink : public LinearFVElementalKernel
{
public:
  static InputParameters validParams();
  LinearWCNSFV2PInterfaceAreaSourceSink(const InputParameters & params);

  virtual Real computeMatrixContribution() override;
  virtual Real computeRightHandSideContribution() override;
  virtual void setCurrentElemInfo(const ElemInfo * elem_info) override;

  /**
   * The terminal velocity of a particle, solved together with its drag coefficient
   * \f$ C_D = 24(1 + 0.1 Re_D^{0.75})/Re_D \f$ from
   *
   * \f[
   *   u_r \left(1 + 0.1 \left(B u_r\right)^{3/4}\right) = T ,
   *   \qquad B = \frac{\rho_f d_b (1-\alpha_g)}{\mu_f} ,
   *   \qquad T = \frac{B}{24}\,\frac{d_b g \Delta\rho}{3 \rho_f} ,
   * \f]
   *
   * by a Newton iteration safeguarded by the bracket \f$ [0, T] \f$.
   *
   * @param stokes_velocity the velocity \f$ T \f$ obtained by ignoring the Reynolds correction
   * @param reynolds_per_velocity the factor \f$ B \f$ converting a velocity into \f$ Re_D \f$
   */
  static Real solveTerminalVelocity(Real stokes_velocity, Real reynolds_per_velocity);

protected:
  /// The closure sets offered by the reference
  enum class ModelEnum
  {
    HIBIKI_ISHII = 0,
    ISHII_KIM = 1
  };

  /// Which closure set to evaluate
  const ModelEnum _model;

  /// The dimension of the simulation
  const unsigned int _dim;

  /// Velocity of the dispersed phase, used for the material derivative of its density
  const Moose::Functor<Real> & _u_var;
  const Moose::Functor<Real> * const _v_var;
  const Moose::Functor<Real> * const _w_var;

  /// Density of the dispersed phase, rho_g
  const Moose::Functor<Real> & _rho_d;
  /// Density of the continuous phase, rho_f
  const Moose::Functor<Real> & _rho_f;
  /// Dynamic viscosity of the continuous phase, only needed by the Ishii and Kim drag coefficient
  const Moose::Functor<Real> * const _mu_f;
  /// Volume fraction of the dispersed phase, alpha_g
  const Moose::Functor<Real> & _f_d;
  /// Surface tension between the phases
  const Moose::Functor<Real> & _sigma;
  /// Turbulent dissipation rate of the continuous phase
  const Moose::Functor<Real> & _epsilon;
  /// Mass transfer rate into the dispersed phase per unit mixture volume
  const Moose::Functor<Real> & _mass_transfer_rate;

  /// Shape factor relating the particle size to the phase fraction and the area, psi
  const Real _shape_factor;
  /// Maximum volume fraction admitted by the model
  const Real _f_d_max;
  /// Gravity vector, only needed by the Ishii and Kim terminal velocity
  const RealVectorValue _gravity;

  /// Hibiki and Ishii closure coefficients
  const Real _gamma_c, _kc, _gamma_b, _kb;
  /// Ishii and Kim closure coefficients
  const Real _c_rc, _c_we, _c_ti, _c, _we_cr;

private:
  /// Evaluates both closure sets, filling the coefficients below
  void computeCoefficients();

  /// Coefficient of the terms that multiply the interfacial area concentration directly
  Real _implicit_coefficient;

  /// The remainder, evaluated from the previous iterate
  Real _lagged_source;
};
