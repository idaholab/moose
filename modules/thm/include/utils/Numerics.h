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
int sgn(T val)
{
  return (T(0) < val) - (val < T(0));
}


/**
 * Compute Reynolds number
 *
 * @param volume_fraction The volume fraction of the phase
 * @param rho The density of the phase
 * @param v The velocity of the phase
 * @param Dh The hydraulic diameter
 * @param visc The viscosity of the phase
 */
Real Reynolds(Real volume_fraction, Real rho, Real v, Real Dh, Real visc);

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
 * @param Dh Hydraulic diameter
 * @param rho_l Density of liquid
 * @param visc_l Viscosity of liquid
 * @return Grashof number
 */
Real Grashof(Real beta, Real dT, Real Dh, Real rho_l, Real visc_l);

/**
 * Compute wall heat transfer coefficient
 * @param Nu Nusselt number
 * @param k Thermal conductivity
 * @param Dh Hydraulic diameter
 * @return Returns the wall heat transfer coefficient
 */
Real wallHeatTransferCoefficient(Real Nu, Real k, Real Dh);

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
int    ireg_vu_SBTL95(double v, double u, double & vt, double & ps, double & ts, double & x, double & v1, double & v2, double & v2t, double & u1, double & u2);

/**
 * Derivative of specific volume wrt alpha_A_liquid (aAL)
 *
 * Makes sense only when using 7-equation model
 * @param sign - The sign (given by the phase)
 * @param arhoA - conserved density of the phase, i.e. alpha_rho_A (liquid or vapor)
 */
Real dv_daAL(Real sign, Real U0);

/**
 * Derivative of specific volume wrt density of the phase (rho_A or alpha_rho_A)
 *
 * @param area - Cross-sectional area
 * @param U0 - conserved density of the phase, i.e. rho_A or alpha_rho_A (liquid or vapor)
 */
Real dv_dU0(Real area, Real U0);

/**
 * Derivative of specific internal energy wrt density of the phase (rho_A or alpha_rho_A)
 *
 * @param U0 - conserved density of the phase, i.e. rho_A or alpha_rho_A (liquid or vapor)
 * @param U1 - conserved momentum of the phase, i.e. rhou_A or alpha_rhou_A (liquid or vapor)
 * @param U2 - conserved total energy of the phase, i.e. rhoE_A or alpha_rhoE_A (liquid or vapor)
 */
Real de_dU0(Real U0, Real U1, Real U2);

/**
 * Derivative of specific internal energy wrt momentum of the phase (rhou_A or alpha_rhou_A)
 *
 * @param U0 - conserved density of the phase, i.e. rho_A or alpha_rho_A (liquid or vapor)
 * @param U1 - conserved momentum of the phase, i.e. rhou_A or alpha_rhou_A (liquid or vapor)
 */
Real de_dU1(Real U0, Real U1);

/**
 * Derivative of specific internal energy wrt total energy of the phase (rhoE_A or alpha_rhoE_A)
 *
 * @param U0 - conserved density of the phase, i.e. rho_A or alpha_rho_A (liquid or vapor)
 */
Real de_dU2(Real U0);

#endif  // NUMERICS_H
