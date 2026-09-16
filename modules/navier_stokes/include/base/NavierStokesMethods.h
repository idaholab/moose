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
inline Real
diffusionStressCoefficient(Real fd, Real rho_d, Real rho_c)
{
  fd = std::clamp(fd, 0.0, 1.0);
  const auto beta_d = fd * rho_d;
  const auto beta_c = (1.0 - fd) * rho_c;
  const auto rho_m = beta_d + beta_c;
  return (rho_m > 0.0) ? beta_d * beta_c / rho_m : 0.0;
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
 * Schiller and Naumann's branch of the linear drag function, valid below the transition.
 *
 * Offered separately from dragFunction so that a solver which has already established from its
 * inputs that the root lies on this branch can evaluate it without re-testing the Reynolds number
 * on every pass. That matters: the two branches of dragFunction do not meet exactly, so an
 * iteration whose intermediate iterates re-tested the transition could step across the seam and
 * oscillate. See LinearWCNSFV2PSlipVelocityFunctorMaterial::solveSlipSpeed.
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
inline Real
schillerNaumannDragDerivative(Real Re_p)
{
  mooseAssert(Re_p >= 0, "The particle Reynolds number is formed from a magnitude");
  return 0.15 * 0.687 * std::pow(Re_p, 0.687);
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
 * The linear drag function of a distorted fluid particle.
 *
 * Above roughly a millimetre a bubble no longer behaves as a rigid sphere: it deforms, and its
 * drag coefficient grows with size rather than falling with Reynolds number,
 * \\f$ C_D = \\frac{2}{3} d_d \\sqrt{g \\Delta\\rho / \\sigma} \\f$. Manninen's closure carries the
 * drag as the linear function \\f$ f_{drag} = C_D Re_p / 24 \\f$, which normalises Stokes drag to
 * unity, so that coefficient becomes
 *
 * \\f[
 *   f_{drag} = \\frac{d_d^2 \\rho_c \\left|u_s\\right|}{36 \\mu_c}
 *              \\sqrt{\\frac{g \\Delta\\rho}{\\sigma}} .
 * \\f]
 *
 * Substituted into the closure this returns the terminal velocity
 * \\f$ \\sqrt{2}\\left(g\\sigma\\Delta\\rho/\\rho_c^2\\right)^{1/4} \\f$, independent of the
 * particle size, which is the drift velocity correlation of Ishii for the bubbly flow regime.
 * See Hibiki and Ishii, Int. J. Heat Mass Transfer 45 (2002) 707, equation (15).
 *
 * Unlike dragFunction this one is linear in the slip speed, so it is returned per unit speed: the
 * caller multiplies by \\f$ \\left|u_s\\right| \\f$.
 *
 * @param particle_diameter diameter of the particles of the dispersed phase
 * @param rho_c continuous phase density
 * @param mu_c continuous phase dynamic viscosity
 * @param delta_rho magnitude of the density difference between the phases
 * @param sigma surface tension between the phases
 * @param gravity_magnitude magnitude of the gravity vector
 */
inline Real
distortedDragFunctionPerSpeed(Real particle_diameter,
                              Real rho_c,
                              Real mu_c,
                              Real delta_rho,
                              Real sigma,
                              Real gravity_magnitude)
{
  return Utility::pow<2>(particle_diameter) * rho_c / (36.0 * mu_c) *
         std::sqrt(gravity_magnitude * delta_rho / sigma);
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
