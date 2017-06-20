#ifndef NUMERICS_H
#define NUMERICS_H

#include "libmesh/libmesh_common.h"

using namespace libMesh;

// gravitational acceleration, or gravity constant
static const Real gravity_const = 9.81;

// Stefan-Boltzman constant, in [W/m^2-K]
static const Real Stefan_Boltzman_const = 5.670e-8;

/**
 * The sign function
 * @param val The argument of the sign function
 * @return -1 for negative values, 0 for zero and 1 for positive values
 */
template <typename T>
int
sgn(T val)
{
  return (T(0) < val) - (val < T(0));
}

/**
 * Compute Reynolds number
 *
 * @param volume_fraction The volume fraction of the phase
 * @param rho The density of the phase
 * @param v The velocity of the phase
 * @param D_h The hydraulic diameter
 * @param visc The viscosity of the phase
 */
Real Reynolds(Real volume_fraction, Real rho, Real v, Real D_h, Real visc);

/**
 * Compute Prandtl number
 * @param cp Specific heat
 * @param mu Dynamic viscosity
 * @param k Thermal conductivity
 * @return
 */
Real Prandtl(Real cp, Real mu, Real k);

/**
 * Compute Grashof number
 * @param beta Thermal expansion coefficient
 * @param dT |T_w - T|
 * @param D_h Hydraulic diameter
 * @param rho_l Density of liquid
 * @param visc_l Viscosity of liquid
 * @return Grashof number
 */
Real Grashof(Real beta, Real dT, Real D_h, Real rho_l, Real visc_l);

/**
 * Compute Laplace number (or coefficient)
 * @param surf_tension Surface tension
 * @param delta_rho Difference in density of phases
 * @return Laplace number
 */
Real Laplace(Real surf_tension, Real delta_rho);

/**
 * Compute viscosity number (or coefficient)
 * @param viscosity Viscosity
 * @param surf_tension Surface tension
 * @param rho_k Density of k-th phase of interest
 * @param delta_rho Density difference
 * @return viscosity number
 */
Real viscosityNumber(Real viscosity, Real surf_tension, Real rho_k, Real delta_rho);

/**
 * Compute wall heat transfer coefficient
 * @param Nu Nusselt number
 * @param k Thermal conductivity
 * @param D_h Hydraulic diameter
 * @return Returns the wall heat transfer coefficient
 */
Real wallHeatTransferCoefficient(Real Nu, Real k, Real D_h);

/**
* Computes surface tension [N/m]
* @param temperature
* @return returns the surface tension [N/m]
*/
Real surfaceTension(Real temperature);

/**
 * Determine the region (liquid, vapor, two phase)
 *
 * Taken from libSBTL 0.9.0 (more optimal for our needs)
 * @param v Specific volume
 * @param u Specific internal energy
 * @param vt "Transformed" specific volume
 * @param ps Stauration pressure
 * @param ts Saturation temperature
 * @return The region
 */
int ireg_vu_SBTL95(double v,
                   double u,
                   double & vt,
                   double & ps,
                   double & ts,
                   double & x,
                   double & v1,
                   double & v2,
                   double & v2t,
                   double & u1,
                   double & u2);

/**
 * Derivative of specific volume wrt alpha_liquid
 *
 * Makes sense only when using 7-equation model
 * @param area - The cross-sectional area
 * @param arhoA - density equation solution variable: alpha*rho*A
 * @param is_liquid - True if the specific volume corresponds to liquid phase
 */
Real dv_dalpha_liquid(Real area, Real arhoA, bool is_liquid);

/**
 * Derivative of specific volume wrt density equation solution variable
 *
 * @param area - Cross-sectional area
 * @param arhoA - density equation solution variable: alpha*rho*A
 */
Real dv_darhoA(Real area, Real arhoA);

/**
 * Derivative of specific internal energy wrt density of the phase (rhoA or arhoA)
 *
 * @param arhoA - density equation solution variable: alpha*rho*A
 * @param arhouA - momentum equation solution variable: alpha*rho*u*A
 * @param arhoEA - energy equation solution variable: alpha*rho*E*A
 */
Real de_darhoA(Real arhoA, Real arhouA, Real arhoEA);

/**
 * Derivative of specific internal energy wrt momentum of the phase (rhouA or arhouA)
 *
 * @param arhoA - density equation solution variable: alpha*rho*A
 * @param arhouA - momentum equation solution variable: alpha*rho*u*A
 */
Real de_darhouA(Real arhoA, Real arhouA);

/**
 * Derivative of specific internal energy wrt total energy of the phase (rhoEA or arhoEA)
 *
 * @param arhoA - density equation solution variable: alpha*rho*A
 */
Real de_darhoEA(Real arhoA);

#endif // NUMERICS_H
