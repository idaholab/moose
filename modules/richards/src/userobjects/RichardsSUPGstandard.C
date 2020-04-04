//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RichardsSUPGstandard.h"
#include "libmesh/utility.h"

registerMooseObject("RichardsApp", RichardsSUPGstandard);

InputParameters
RichardsSUPGstandard::validParams()
{
  InputParameters params = RichardsSUPG::validParams();
  params.addRequiredRangeCheckedParam<Real>(
      "p_SUPG",
      "p_SUPG > 0",
      "SUPG pressure.  This parameter controls the strength of the "
      "upwinding.  This parameter must be positive.  If you need to track "
      "advancing fronts in a problem, then set to less than your expected "
      "range of pressures in your unsaturated zone.  Otherwise, set "
      "larger, and then minimal upwinding will occur and convergence will "
      "typically be good.  If you need no SUPG, it is more efficient not "
      "to use this UserObject.");
  params.addClassDescription(
      "Standard SUPG relationships for Richards flow based on Appendix A of      TJR Hughes, M "
      "Mallet and A Mizukami ``A new finite element formulation for computational fluid dynamics:: "
      "II. Behond SUPG'' Computer Methods in Applied Mechanics and Engineering 54 (1986) 341--355");
  return params;
}

RichardsSUPGstandard::RichardsSUPGstandard(const InputParameters & parameters)
  : RichardsSUPG(parameters), _p_SUPG(getParam<Real>("p_SUPG"))
{
}

RealVectorValue
RichardsSUPGstandard::velSUPG(RealTensorValue perm,
                              RealVectorValue gradp,
                              Real density,
                              RealVectorValue gravity) const
{
  return -perm * (gradp - density * gravity); // points in direction of info propagation
}

RealTensorValue
RichardsSUPGstandard::dvelSUPG_dgradp(RealTensorValue perm) const
{
  return -perm;
}

RealVectorValue
RichardsSUPGstandard::dvelSUPG_dp(RealTensorValue perm,
                                  Real density_prime,
                                  RealVectorValue gravity) const
{
  return perm * density_prime * gravity;
}

Real
RichardsSUPGstandard::cosh_relation(Real alpha) const
{
  if (alpha >= 5.0 || alpha <= -5.0)
    return ((alpha > 0.0) ? 1.0 : -1.0) - 1.0 / alpha; // prevents overflows
  else if (alpha == 0)
    return 0.0;
  return 1.0 / std::tanh(alpha) - 1.0 / alpha;
}

Real
RichardsSUPGstandard::cosh_relation_prime(Real alpha) const
{
  if (alpha >= 5.0 || alpha <= -5.0)
    return 1.0 / (alpha * alpha); // prevents overflows
  else if (alpha == 0)
    return 1.0 / 3.0;
  return 1.0 - Utility::pow<2>(std::cosh(alpha) / std::sinh(alpha)) + 1.0 / (alpha * alpha);
}

// the following is physically 2*velocity/element_length
RealVectorValue
RichardsSUPGstandard::bb(RealVectorValue vel,
                         int dimen,
                         RealVectorValue xi_prime,
                         RealVectorValue eta_prime,
                         RealVectorValue zeta_prime) const
{
  RealVectorValue b;
  b(0) = xi_prime * vel;
  if (dimen >= 2)
    b(1) = eta_prime * vel;
  if (dimen == 3)
    b(2) = zeta_prime * vel;
  return b;
}

// following is d(bb*bb)/d(gradp)
RealVectorValue
RichardsSUPGstandard::dbb2_dgradp(RealVectorValue vel,
                                  RealTensorValue dvel_dgradp,
                                  RealVectorValue xi_prime,
                                  RealVectorValue eta_prime,
                                  RealVectorValue zeta_prime) const
{
  // if dvel_dgradp is symmetric so transpose is probably a waste of time
  return 2.0 * ((xi_prime * vel) * (dvel_dgradp.transpose() * xi_prime) +
                (eta_prime * vel) * (dvel_dgradp.transpose() * eta_prime) +
                (zeta_prime * vel) * (dvel_dgradp.transpose() * zeta_prime));
}

// following is d(bb*bb)/d(p)
Real
RichardsSUPGstandard::dbb2_dp(RealVectorValue vel,
                              RealVectorValue dvel_dp,
                              RealVectorValue xi_prime,
                              RealVectorValue eta_prime,
                              RealVectorValue zeta_prime) const
{
  return 2.0 *
         ((xi_prime * vel) * (dvel_dp * xi_prime) + (eta_prime * vel) * (dvel_dp * eta_prime) +
          (zeta_prime * vel) * (dvel_dp * zeta_prime));
}

Real
RichardsSUPGstandard::tauSUPG(RealVectorValue vel, Real traceperm, RealVectorValue b) const
{
  // vel = velocity, b = bb
  Real norm_v = vel.norm();
  Real norm_b = b.norm(); // Hughes et al investigate infinity-norm and 2-norm.  i just use 2-norm
                          // here.   norm_b ~ 2|a|/ele_length_in_direction_of_a

  if (norm_b == 0)
    return 0.0; // Only way for norm_b=0 is for zero ele size, or vel=0.  Either way we don't have
                // to upwind.

  Real h = 2.0 * norm_v / norm_b; // h is a measure of the element length in the "a" direction
  Real alpha = 0.5 * norm_v * h / (traceperm * _p_SUPG); // this is the Peclet number

  const Real xi_tilde = RichardsSUPGstandard::cosh_relation(alpha);

  return xi_tilde / norm_b;
}

RealVectorValue
RichardsSUPGstandard::dtauSUPG_dgradp(RealVectorValue vel,
                                      RealTensorValue dvel_dgradp,
                                      Real traceperm,
                                      RealVectorValue b,
                                      RealVectorValue db2_dgradp) const
{
  Real norm_vel = vel.norm();
  if (norm_vel == 0)
    return RealVectorValue();

  RealVectorValue norm_vel_dgradp(dvel_dgradp * vel / norm_vel);

  Real norm_b = b.norm();
  if (norm_b == 0)
    return RealVectorValue();
  RealVectorValue norm_b_dgradp = db2_dgradp / 2.0 / norm_b;

  Real h = 2 * norm_vel / norm_b; // h is a measure of the element length in the "a" direction
  RealVectorValue h_dgradp(2 * norm_vel_dgradp / norm_b -
                           2.0 * norm_vel * norm_b_dgradp / norm_b / norm_b);

  Real alpha = 0.5 * norm_vel * h / traceperm / _p_SUPG; // this is the Peclet number
  RealVectorValue alpha_dgradp =
      0.5 * (norm_vel_dgradp * h + norm_vel * h_dgradp) / traceperm / _p_SUPG;

  Real xi_tilde = RichardsSUPGstandard::cosh_relation(alpha);
  Real xi_tilde_prime = RichardsSUPGstandard::cosh_relation_prime(alpha);
  RealVectorValue xi_tilde_dgradp = xi_tilde_prime * alpha_dgradp;

  RealVectorValue tau_dgradp =
      xi_tilde_dgradp / norm_b - xi_tilde * norm_b_dgradp / (norm_b * norm_b);

  return tau_dgradp;
}

Real
RichardsSUPGstandard::dtauSUPG_dp(RealVectorValue vel,
                                  RealVectorValue dvel_dp,
                                  Real traceperm,
                                  RealVectorValue b,
                                  Real db2_dp) const
{
  Real norm_vel = vel.norm();
  if (norm_vel == 0.0)
    return 0.0; // this deriv is not necessarily correct, but i can't see a better thing to do

  Real norm_vel_dp = dvel_dp * vel / norm_vel;

  Real norm_b = b.norm();
  if (norm_b == 0)
    return 0.0; // this deriv is not necessarily correct, but i can't see a better thing to do
  Real norm_b_dp = db2_dp / (2.0 * norm_b);

  Real h = 2.0 * norm_vel / norm_b; // h is a measure of the element length in the "a" direction
  Real h_dp = 2.0 * norm_vel_dp / norm_b - 2.0 * norm_vel * norm_b_dp / (norm_b * norm_b);

  Real alpha = 0.5 * norm_vel * h / (traceperm * _p_SUPG); // this is the Peclet number
  Real alpha_dp = 0.5 * (norm_vel_dp * h + norm_vel * h_dp) / (traceperm * _p_SUPG);

  Real xi_tilde = RichardsSUPGstandard::cosh_relation(alpha);
  Real xi_tilde_prime = RichardsSUPGstandard::cosh_relation_prime(alpha);
  Real xi_tilde_dp = xi_tilde_prime * alpha_dp;

  // Real tau = xi_tilde/norm_b;
  const Real tau_dp = xi_tilde_dp / norm_b - xi_tilde * norm_b_dp / (norm_b * norm_b);

  return tau_dp;
}

bool
RichardsSUPGstandard::SUPG_trivial() const
{
  return false;
}
