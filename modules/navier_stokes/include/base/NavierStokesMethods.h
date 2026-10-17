//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include <vector>
#include <utility>
#include <set>
#include <algorithm>
#include "Moose.h"
#include "FaceInfo.h"
#include "MooseUtils.h"
#include "libmesh/utility.h"
#include "ADReal.h"
#include "metaphysicl/raw_type.h"
#include "FEProblemBase.h"
#include "SubProblem.h"

namespace NS
{
/**
 * Delta function, which returns zero if $i\ne j$ and unity if $i=j$
 * @param[in] i   integer number
 * @param[in] j   integer number
 * @return delta function
 */
int delta(unsigned int i, unsigned int j);

/**
 * Sign function, returns $+1$ if $a$ is positive and $-1$ if $a$ is negative
 * @param[in] a   number
 * @return the sign of the input
 */
int computeSign(const Real & a);

/**
 * Determines the index $i$ in a sorted array such that the input point is within
 * the $i$-th and $i+1$-th entries in the array.
 * @param[in] p      input point
 * @param[in] bounds sorted array
 * @return index of point
 */
unsigned int getIndex(const Real & p, const std::vector<Real> & bounds);

/**
 * Computes the derivative of the Reynolds number, $Re\equiv \frac{\rho Vd}{\mu}$,
 * with respect to an arbitrary variable $\zeta$, where it is assumed that only the
 * material properties of density $\rho$ and dynamic viscosity $\mu$ have nonzero
 * derivatives with respect to $\zeta$. To eliminate the need to pass in the velocity $V$ and
 * characteristic length $d$, the derivative is rewritten in terms of the Reynolds
 * number such that the partial derivative of $Re$ with respect to an aritrary
 * parameter $\zeta$ is
 *
 * $\frac{\partial Re}{\partial\zeta}=Re\left(\frac{1}{\rho}\frac{\partial\rho}{\partial
 * x}-\frac{1}{\mu}\frac{\partial\mu}{\partial x}$
 *
 * @param[in] Re   Reynolds number
 * @param[in] rho  density
 * @param[in] mu   dynamic viscosity
 * @param[in] drho partial derivative of density with respect to arbitrary variable $\zeta$
 * @param[in] dmu  partial derivative of dynamic viscosity with respect to arbitrary variable
 * $\zeta$
 * @return derivative of Reynolds number with respect to $\zeta$
 */
Real reynoldsPropertyDerivative(
    const Real & Re, const Real & rho, const Real & mu, const Real & drho, const Real & dmu);

/**
 * Computes the derivative of the Prandtl number, $Pr\equiv\frac{\mu C_p}{k}$, with respect
 * to an arbitrary variale $\zeta$. This derivative is
 *
 * $\frac{\partial Pr}{\partial \zeta}=\frac{k\left(\mu\frac{\partial
 * C_p}{\partial\zeta}+C_p\frac{\partial mu}{\partial\zeta}\right)-\mu C_p\frac{\partial
 * k}{\partial\zeta}}{k^2}$
 *
 * @param[in] mu  dynamic viscosity
 * @param[in] cp  isobaric specific heat
 * @param[in] k   thermal conductivity
 * @param[in] dmu derivative of dynamic viscosity with respect to $\zeta$
 * @param[in] dcp derivative of isobaric specific heat with respect to $\zeta$
 * @param[in] dk  derivative of thermal conductivity with respect to $\zeta$
 * @return derivative of Prandtl number with respect to $\zeta$
 */
Real prandtlPropertyDerivative(const Real & mu,
                               const Real & cp,
                               const Real & k,
                               const Real & dmu,
                               const Real & dcp,
                               const Real & dk);

/**
 * Finds the friction velocity using standard velocity wall functions formulation.
 * It is used in WallFunctionWallShearStressAux, WallFunctionYPlusAux and
 * INSFVWallFunctionBC.
 * @param mu the dynamic viscosity
 * @param rho the density
 * @param u the centroid velocity
 * @param dist the element centroid distance to the wall
 * @return the velocity at the wall
 */
template <typename T>
T findUStar(const T & mu, const T & rho, const T & u, Real dist);

/**
 * Finds the non-dimensional wall distance normalized with the friction velocity
 * Implements a fixed-point iteration in the wall function to get this velocity
 * @param mu the dynamic viscosity
 * @param rho the density
 * @param u the centroid velocity
 * @param dist the element centroid distance to the wall
 * @return the non-dimensional wall distance
 */
template <typename T>
T findyPlus(const T & mu, const T & rho, const T & u, Real dist);

using MooseUtils::isZero;

/**
 * The coefficient of the diffusion stress of the mixture model, \f$ \beta_d \beta_c / \rho_m \f$,
 * which also weights the enthalpy the relative motion carries. The phase fraction is clamped into
 * [0, 1], as the mixture property material clamps it. Zero where either phase is absent.
 */
template <typename T>
T
diffusionStressCoefficient(const T & fd_in, const T & rho_d, const T & rho_c)
{
  const T fd = (fd_in < 0.0) ? T(0.0) : ((fd_in > 1.0) ? T(1.0) : fd_in);
  const auto beta_d = fd * rho_d;
  const auto beta_c = (1.0 - fd) * rho_c;
  const auto rho_m = beta_d + beta_c;
  return (rho_m > 0.0) ? T(beta_d * beta_c / rho_m) : T(0.0);
}

/**
 * Whether the dispersed phase may cross a face: any internal face, and a boundary face only if one
 * of the sidesets the face belongs to is in `slip_boundaries`. Every sideset of the face is
 * checked, the permeable one need not be the first the mesh lists.
 */
inline bool
slipAllowedOnFace(const FaceInfo & fi, const std::set<BoundaryID> & slip_boundaries)
{
  if (fi.neighborPtr())
    return true;
  const auto & ids = fi.boundaryIDs();
  return std::any_of(ids.begin(),
                     ids.end(),
                     [&slip_boundaries](const auto id) { return slip_boundaries.count(id); });
}

/**
 * Assembles the slip velocity vector of a dispersed phase from its component functors. The
 * components a lower dimensional mesh does not carry are passed as null and read as zero.
 */
template <typename T, typename SpaceArg>
libMesh::VectorValue<T>
slipVelocityVector(const Moose::Functor<T> & u_slip,
                   const Moose::Functor<T> * v_slip,
                   const Moose::Functor<T> * w_slip,
                   const SpaceArg & arg,
                   const Moose::StateArg & state)
{
  libMesh::VectorValue<T> slip(u_slip(arg, state), 0.0, 0.0);
  if (v_slip)
    slip(1) = (*v_slip)(arg, state);
  if (w_slip)
    slip(2) = (*w_slip)(arg, state);
  return slip;
}

/**
 * Checks that a slip velocity was given a component for every dimension of the mesh, the
 * u component being required by the parameters of every object taking one.
 */
inline void
checkSlipVelocityComponents(const MooseObject & object, unsigned int dim, bool has_v, bool has_w)
{
  if (dim >= 2 && !has_v)
    object.paramError("v_slip", "In two or more dimensions, the v_slip velocity must be supplied");
  if (dim >= 3 && !has_w)
    object.paramError("w_slip", "In three dimensions, the w_slip velocity must be supplied");
}

/**
 * Schiller and Naumann's branch of the linear drag function, valid below the transition. Offered
 * separately from dragFunction for a solver that has already established which branch its root
 * lies on; see solveSlipSpeed.
 */
template <typename T>
T
schillerNaumannDragFunction(const T & Re_p)
{
  using std::pow;
  mooseAssert(MetaPhysicL::raw_value(Re_p) >= 0,
              "The particle Reynolds number is formed from a magnitude");
  return 1.0 + 0.15 * pow(Re_p, 0.687);
}

/**
 * Re_p times the derivative of schillerNaumannDragFunction with respect to Re_p, written as the
 * product because \f$ Re_p \, f'(Re_p) \f$ is finite at the origin while \f$ f' \f$ alone is not.
 */
template <typename T>
T
schillerNaumannDragDerivative(const T & Re_p)
{
  using std::pow;
  mooseAssert(MetaPhysicL::raw_value(Re_p) >= 0,
              "The particle Reynolds number is formed from a magnitude");
  return 0.15 * 0.687 * pow(Re_p, 0.687);
}

/**
 * The linear drag function of a dispersed phase: Schiller and Naumann's correlation below the
 * transition and Newton's regime above it. ANSYS Fluent Theory Guide equation 16.4-14.
 *
 * The function is strictly increasing, which is what makes the slip velocity solvable from the
 * force balance. It is very nearly, but not exactly, continuous: at the transition the two
 * branches give 18.262 and 18.300, a step of about 0.2%. Any iteration that evaluates this
 * function must therefore be bracketed, or must select its branch in advance from a quantity that
 * does not change as the iteration proceeds.
 */
template <typename T>
T
dragFunction(const T & Re_p)
{
  mooseAssert(MetaPhysicL::raw_value(Re_p) >= 0,
              "The particle Reynolds number is formed from a magnitude");
  return (Re_p <= 1000.0) ? schillerNaumannDragFunction(Re_p) : 0.0183 * Re_p;
}

/**
 * Solves g(x) = 0 for a strictly increasing g whose root is known to lie in [lower, upper]. Newton
 * steps are taken from the initial iterate x while they stay inside the bracket, which the sign of
 * the residual shrinks around the root; a step that would leave it is replaced by bisection. The
 * comparisons use raw values, so that an automatic differentiation type carries its derivatives
 * through the arithmetic.
 * @param residual_and_derivative callable returning the pair (g(x), g'(x))
 * @param scale the magnitude the residual is measured against for convergence
 */
template <typename T, typename Function>
T
bracketedNewton(const Function & residual_and_derivative, T lower, T upper, T x, const Real scale)
{
  // The residual falls below this relative tolerance in a handful of iterations; the cap is a
  // backstop, not the expected exit
  constexpr Real rel_tol = 1e-12;
  constexpr unsigned int max_its = 50;

  for ([[maybe_unused]] const auto it : make_range(max_its))
  {
    const auto [residual, derivative] = residual_and_derivative(x);

    if (std::abs(MetaPhysicL::raw_value(residual)) <= rel_tol * scale)
      break;

    if (MetaPhysicL::raw_value(residual) > 0.0)
      upper = x;
    else
      lower = x;

    // Fall back on bisection if Newton steps outside the bracket
    const T candidate = x - residual / derivative;
    x = (MetaPhysicL::raw_value(candidate) > MetaPhysicL::raw_value(lower) &&
         MetaPhysicL::raw_value(candidate) < MetaPhysicL::raw_value(upper))
            ? candidate
            : T(0.5 * (lower + upper));
  }

  return x;
}

/**
 * Solves Manninen's force balance together with the drag correlation for the slip speed:
 * \f$ s \, f(R s) = s_0 \f$, where \f$ s_0 \f$ is the slip speed in the Stokes limit and
 * \f$ R \f$ converts a speed into a particle Reynolds number. The left hand side is zero at
 * \f$ s = 0 \f$ and strictly increasing, so the root is unique, and since \f$ f \ge 1 \f$ it is
 * bracketed by \f$ [0, s_0] \f$. Solved by a Newton iteration safeguarded by that bracket, and
 * templated so that both slip closures can call it.
 */
template <typename T>
T
solveSlipSpeed(const T & stokes_speed, const T & reynolds_per_speed)
{
  using std::sqrt;
  // f(0) = 1, so a vanishing acceleration gives a vanishing slip and the drag never enters
  if (MetaPhysicL::raw_value(stokes_speed) <= 0.0 ||
      MetaPhysicL::raw_value(reynolds_per_speed) <= 0.0)
    return stokes_speed;

  // Solve in Reynolds number rather than in speed: multiplying s f(R s) = s0 through by R gives
  // Re f(Re) = B, with B = R s0 formed entirely from inputs, so the branch of f is chosen before
  // iterating rather than re-tested on every pass.
  const T driving_group = reynolds_per_speed * stokes_speed;

  // Below this the drag correction 0.15 Re^0.687 is 3e-13, smaller than the relative tolerance the
  // loop would converge to, so the Stokes answer is already the converged one.
  constexpr Real negligible_driving_group = 1e-17;
  if (MetaPhysicL::raw_value(driving_group) < negligible_driving_group)
    return stokes_speed;

  // In Newton's regime f = 0.0183 Re, so the balance becomes 0.0183 Re^2 = B and is exact. The
  // branches of dragFunction change over at Re = 1000, which is this value of B.
  constexpr Real newton_regime_driving_group = 0.0183 * 1000.0 * 1000.0;
  if (MetaPhysicL::raw_value(driving_group) >= newton_regime_driving_group)
    return sqrt(driving_group / 0.0183) / reynolds_per_speed;

  // g(Re) = Re f(Re) is zero at the origin and strictly increasing, and f >= 1 puts the root in
  // [0, B]. The iteration starts from one Picard step off the Stokes guess Re = B.
  const T reynolds = NS::bracketedNewton(
      [&driving_group](const T & re)
      {
        // d/dRe [Re f(Re)] = f(Re) + Re f'(Re), the second term written as the finite product
        const T drag = NS::schillerNaumannDragFunction(re);
        return std::pair<T, T>(re * drag - driving_group,
                               drag + NS::schillerNaumannDragDerivative(re));
      },
      T(0.0),
      driving_group,
      driving_group / NS::schillerNaumannDragFunction(driving_group),
      MetaPhysicL::raw_value(driving_group));

  return reynolds / reynolds_per_speed;
}

/**
 * The bubble interaction sources of the one-group interfacial area transport equation in the
 * closure of Hibiki and Ishii, Int. J. Heat Mass Transfer 43 (2000) 2711: coalescence by random
 * collision and breakage by turbulent impact. There is no wake entrainment term in this closure.
 *
 * Both share the prefactor
 * \f[
 *   \Pi = \left(\frac{\alpha_g}{\chi_p}\right)^{2}
 *         \frac{u_t}{d_b^{11/3}\left(\alpha_{g,max}-\alpha_g\right)}
 * \f]
 * in which the packing factor divides: both terms grow without bound as the dispersed phase
 * approaches its maximum, which is what crowding does to collision. The reference forms the
 * turbulent velocity scale as \f$ u_t = \epsilon^{1/3} \f$ from the dissipation; it is taken as an
 * argument here so that a caller estimating it another way still evaluates the same closure.
 *
 * Written once and called from both discretizations, so that they cannot disagree on it.
 *
 * @param f_d volume fraction of the dispersed phase
 * @param f_d_over_xi that fraction divided by the interfacial area concentration, guarded by the
 *   caller however it sees fit
 * @param d_b averaged particle size
 * @param u_t turbulent velocity scale
 * @param packing the margin to the maximum packing fraction, floored by the caller
 * @param rho_f continuous phase density
 * @param sigma surface tension
 * @param gamma_c, k_c coalescence coefficients
 * @param gamma_b, k_b breakage coefficients
 * @return the pair (S_RC, S_TI) as sources of the area equation: the coalescence sink, which is
 *   non-positive, and the breakage source, which is non-negative
 */
template <typename T>
std::pair<T, T>
hibikiIshiiAreaSources(const T & f_d,
                       const T & f_d_over_xi,
                       const T & d_b,
                       const T & u_t,
                       const T & packing,
                       const T & rho_f,
                       const T & sigma,
                       const Real gamma_c,
                       const Real k_c,
                       const Real gamma_b,
                       const Real k_b)
{
  using std::exp;
  using std::pow;
  using std::sqrt;

  const T prefactor = Utility::pow<2>(f_d_over_xi) * u_t / (pow(d_b, 11. / 3.) * packing);
  const T s_rc = -prefactor * gamma_c * Utility::pow<2>(f_d) *
                 exp(-k_c * pow(d_b, 5. / 6.) * sqrt(rho_f / sigma) * u_t);
  const T s_ti = prefactor * gamma_b * f_d * (1.0 - f_d) *
                 exp(-k_b * sigma / (rho_f * pow(d_b, 5. / 3.) * Utility::pow<2>(u_t)));
  return {s_rc, s_ti};
}

/**
 * The linear drag function of a distorted fluid particle, the deformed-bubble regime in which the
 * drag coefficient grows with size rather than falling with Reynolds number. Substituted into the
 * closure it gives Ishii's terminal velocity for bubbly flow, independent of the particle size; the
 * derivation is on the LinearWCNSFV2PSlipVelocityFunctorMaterial documentation page.
 *
 * Unlike dragFunction this one is linear in the slip speed, so it is returned per unit speed: the
 * caller multiplies by \f$ \left|u_s\right| \f$.
 *
 * @param particle_diameter diameter of the particles of the dispersed phase
 * @param rho_c continuous phase density
 * @param mu_c continuous phase dynamic viscosity
 * @param delta_rho magnitude of the density difference between the phases
 * @param sigma surface tension between the phases
 * @param gravity_magnitude magnitude of the gravity vector
 */
template <typename T>
inline T
distortedDragFunctionPerSpeed(const T & particle_diameter,
                              const T & rho_c,
                              const T & mu_c,
                              const T & delta_rho,
                              const T & sigma,
                              const Real gravity_magnitude)
{
  using std::sqrt;
  return Utility::pow<2>(particle_diameter) * rho_c / (36.0 * mu_c) *
         sqrt(gravity_magnitude * delta_rho / sigma);
}

/**
 * Compute the speed (velocity norm) given the supplied velocity
 */
template <typename T>
T computeSpeed(const libMesh::VectorValue<T> & velocity);

/**
 * Utility function to compute the shear strain rate
 */
template <typename T>
T computeShearStrainRateNormSquared(const Moose::Functor<T> & u,
                                    const Moose::Functor<T> * v,
                                    const Moose::Functor<T> * w,
                                    const Moose::ElemArg & elem_arg,
                                    const Moose::StateArg & state,
                                    const Moose::CoordinateSystemType coord_sys = Moose::COORD_XYZ,
                                    const unsigned int rz_radial_coord = 0);

/**
 * Map marking wall bounded elements
 * The map passed in \p wall_bounded_map gets cleared and re-populated
 */
void getWallBoundedElements(const std::vector<BoundaryName> & wall_boundary_name,
                            const FEProblemBase & fe_problem,
                            const SubProblem & subproblem,
                            const std::set<SubdomainID> & block_ids,
                            std::unordered_set<const Elem *> & wall_bounded);

/**
 * Map storing wall ditance for near-wall marked elements
 * The map passed in \p dist_map gets cleared and re-populated
 */
void getWallDistance(const std::vector<BoundaryName> & wall_boundary_name,
                     const FEProblemBase & fe_problem,
                     const SubProblem & subproblem,
                     const std::set<SubdomainID> & block_ids,
                     std::map<const Elem *, std::vector<Real>> & dist_map);

/**
 * Map storing face arguments to wall bounded faces
 * The map passed in \p face_info_map gets cleared and re-populated
 */
void getElementFaceArgs(const std::vector<BoundaryName> & wall_boundary_name,
                        const FEProblemBase & fe_problem,
                        const SubProblem & subproblem,
                        const std::set<SubdomainID> & block_ids,
                        std::map<const Elem *, std::vector<const FaceInfo *>> & face_info_map);

/**
 * Compute the divergence of a vector given its matrix of derivatives
 */
template <typename T, typename VectorType, typename PointType>
T
divergence(const TensorValue<T> & gradient,
           const VectorType & value,
           const PointType & point,
           const Moose::CoordinateSystemType & coord_sys,
           const unsigned int rz_radial_coord)
{
  mooseAssert((coord_sys == Moose::COORD_XYZ) || (coord_sys == Moose::COORD_RZ),
              "This function only supports calculations of divergence in Cartesian and "
              "axisymmetric coordinate systems");
  auto div = gradient.tr();
  if (coord_sys == Moose::COORD_RZ)
    // u_r / r
    div += value(rz_radial_coord) / point(rz_radial_coord);
  return div;
}

/**
 * Compute wall heat transfer coefficient
 * @param Nu Nusselt number
 * @param k Thermal conductivity
 * @param D_h Hydraulic diameter
 *
 * @return the wall heat transfer coefficient
 */
template <typename T1, typename T2, typename T3>
auto
wallHeatTransferCoefficient(const T1 & Nu, const T2 & k, const T3 & D_h)
{
  return Nu * k / D_h;
}

// Prevent implicit instantiation in other translation units where these classes are used
extern template Real
findUStar<Real>(const Real & mu, const Real & rho, const Real & u, const Real dist);
extern template ADReal
findUStar<ADReal>(const ADReal & mu, const ADReal & rho, const ADReal & u, const Real dist);

extern template Real findyPlus<Real>(const Real & mu, const Real & rho, const Real & u, Real dist);
extern template ADReal
findyPlus<ADReal>(const ADReal & mu, const ADReal & rho, const ADReal & u, Real dist);

extern template Real computeSpeed<Real>(const libMesh::VectorValue<Real> & velocity);
extern template ADReal computeSpeed<ADReal>(const libMesh::VectorValue<ADReal> & velocity);

extern template Real
computeShearStrainRateNormSquared<Real>(const Moose::Functor<Real> & u,
                                        const Moose::Functor<Real> * v,
                                        const Moose::Functor<Real> * w,
                                        const Moose::ElemArg & elem_arg,
                                        const Moose::StateArg & state,
                                        const Moose::CoordinateSystemType coord_sys,
                                        const unsigned int rz_radial_coord);
extern template ADReal
computeShearStrainRateNormSquared<ADReal>(const Moose::Functor<ADReal> & u,
                                          const Moose::Functor<ADReal> * v,
                                          const Moose::Functor<ADReal> * w,
                                          const Moose::ElemArg & elem_arg,
                                          const Moose::StateArg & state,
                                          const Moose::CoordinateSystemType coord_sys,
                                          const unsigned int rz_radial_coord);
}
