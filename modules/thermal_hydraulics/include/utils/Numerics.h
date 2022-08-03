//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "libmesh/libmesh_common.h"
#include "libmesh/vector_value.h"
#include "libmesh/dense_vector.h"

#include "ADReal.h"

using namespace libMesh;

namespace THM
{

// Default value for magnitude of acceleration due to gravity
static const Real gravity_const = 9.81;

// Default value for gravitational acceleration vector
static VectorValue<Real> default_gravity_vector = VectorValue<Real>(0.0, 0.0, -gravity_const);

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
 * Tests if two real-valued vectors are equal within some absolute tolerance
 *
 * @param[in] a     First vector
 * @param[in] b     Second vector
 * @param[in] tol   Absolute tolerance
 */
bool absoluteFuzzyEqualVectors(const RealVectorValue & a,
                               const RealVectorValue & b,
                               const Real & tol = libMesh::TOLERANCE * libMesh::TOLERANCE);

/**
 * Tests if two real-valued vectors are parallel within some absolute tolerance
 *
 * @param[in] a     First vector
 * @param[in] b     Second vector
 * @param[in] tol   Absolute tolerance
 */
bool areParallelVectors(const RealVectorValue & a,
                        const RealVectorValue & b,
                        const Real & tol = libMesh::TOLERANCE * libMesh::TOLERANCE);

/**
 * Tests if two real-valued vectors are in the same direction
 *
 * @param[in] a     First vector
 * @param[in] b     Second vector
 * @param[in] tol   Absolute tolerance
 */
bool haveSameDirection(const RealVectorValue & a,
                       const RealVectorValue & b,
                       const Real & tol = libMesh::TOLERANCE * libMesh::TOLERANCE);

/**
 * Computes a derivative of a fraction using quotient rule for a derivative
 * w.r.t. a scalar quantity
 *
 * @param[in] num       numerator value
 * @param[in] den       denominator value
 * @param[in] dnum_dy   derivative of numerator value
 * @param[in] dden_dy   derivative of denominator value
 */
Real
applyQuotientRule(const Real & num, const Real & den, const Real & dnum_dy, const Real & dden_dy);

/**
 * Computes a derivative of a fraction using quotient rule for a derivative
 * w.r.t. a vector quantity
 *
 * @param[in] num       numerator value
 * @param[in] den       denominator value
 * @param[in] dnum_dy   derivative of numerator value
 * @param[in] dden_dy   derivative of denominator value
 */
DenseVector<Real> applyQuotientRule(const Real & num,
                                    const Real & den,
                                    const DenseVector<Real> & dnum_dy,
                                    const DenseVector<Real> & dden_dy);

/**
 * Compute Reynolds number
 *
 * @param volume_fraction The volume fraction of the phase
 * @param rho The density of the phase
 * @param vel The velocity of the phase
 * @param D_h The hydraulic diameter
 * @param mu The viscosity of the phase
 */
Real Reynolds(Real volume_fraction, Real rho, Real vel, Real D_h, Real mu);
ADReal Reynolds(ADReal volume_fraction, ADReal rho, ADReal vel, ADReal D_h, ADReal mu);

/**
 * Compute Prandtl number
 * @param cp Specific heat
 * @param mu Dynamic viscosity
 * @param k Thermal conductivity
 * @return
 */
Real Prandtl(Real cp, Real mu, Real k);
ADReal Prandtl(ADReal cp, ADReal mu, ADReal k);

/**
 * Compute Grashof number
 *
 * @param beta Thermal expansion coefficient
 * @param dT |T_w - T|
 * @param D_h Hydraulic diameter
 * @param rho_liquid Density of liquid
 * @param mu_liquid Viscosity of liquid
 * @param gravity_magnitude   Gravitational acceleration magnitude
 *
 * @return Grashof number
 */
Real Grashof(Real beta, Real dT, Real D_h, Real rho_liquid, Real mu_liquid, Real gravity_magnitude);
ADReal Grashof(ADReal beta,
               ADReal dT,
               ADReal D_h,
               ADReal rho_liquid,
               ADReal mu_liquid,
               Real gravity_magnitude);

/**
 * Compute Laplace number (or coefficient)
 *
 * @param surf_tension Surface tension
 * @param delta_rho Difference in density of phases
 * @param gravity_magnitude   Gravitational acceleration magnitude
 *
 * @return Laplace number
 */
Real Laplace(Real surf_tension, Real delta_rho, Real gravity_magnitude);
ADReal Laplace(ADReal surf_tension, ADReal delta_rho, Real gravity_magnitude);

/**
 * Compute viscosity number (or coefficient)
 *
 * @param viscosity Viscosity
 * @param surf_tension Surface tension
 * @param rho_k Density of k-th phase of interest
 * @param delta_rho Density difference
 * @param gravity_magnitude   Gravitational acceleration magnitude
 *
 * @return viscosity number
 */
Real viscosityNumber(
    Real viscosity, Real surf_tension, Real rho_k, Real delta_rho, Real gravity_magnitude);
ADReal viscosityNumber(
    ADReal viscosity, ADReal surf_tension, ADReal rho_k, ADReal delta_rho, Real gravity_magnitude);

/**
 * Compute wall heat transfer coefficient
 * @param Nu Nusselt number
 * @param k Thermal conductivity
 * @param D_h Hydraulic diameter
 * @return the wall heat transfer coefficient
 */
Real wallHeatTransferCoefficient(Real Nu, Real k, Real D_h);
ADReal wallHeatTransferCoefficient(ADReal Nu, ADReal k, ADReal D_h);

/**
 * Compute Dean number
 * @param Re Reynolds number
 * @param doD tube diameter to coil diameter ratio
 * @return Dean number
 */
Real Dean(Real Re, Real doD);
ADReal Dean(ADReal Re, ADReal doD);

/**
 * Computes velocity and its derivatives from alpha*rho*A and alpha*rho*u*A
 *
 * @param[in] arhoA           alpha*rho*A
 * @param[in] arhouA          alpha*rho*u*A
 * @param[out] vel            velocity
 * @param[out] dvel_darhoA    derivative of velocity w.r.t. alpha*rho*A
 * @param[out] dvel_darhouA   derivative of velocity w.r.t. alpha*rho*u*A
 */
void
vel_from_arhoA_arhouA(Real arhoA, Real arhouA, Real & vel, Real & dvel_darhoA, Real & dvel_darhouA);

/**
 * Computes velocity from alpha*rho*A and alpha*rho*u*A
 *
 * @param arhoA           alpha*rho*A
 * @param arhouA          alpha*rho*u*A
 * @return velocity
 */
ADReal vel_from_arhoA_arhouA(ADReal arhoA, ADReal arhouA);

/**
 * Derivative of velocity w.r.t. alpha*rho*A
 *
 * @param[in] arhoA    alpha*rho*A
 * @param[in] arhouA   alpha*rho*u*A
 * @returns derivative of velocity w.r.t. alpha*rho*A
 */
Real dvel_darhoA(Real arhoA, Real arhouA);

/**
 * Derivative of velocity w.r.t. alpha*rho*u*A
 *
 * @param[in] arhoA   alpha*rho*A
 * @returns derivative of velocity w.r.t. alpha*rho*u*A
 */
Real dvel_darhouA(Real arhoA);

/**
 * Computes density and its derivatives from alpha*rho*A, alpha, and area.
 *
 * @param[in] arhoA   alpha*rho*A
 * @param[in] alpha   volume fraction
 * @param[in] A       area
 * @param[out] rho           density
 * @param[out] drho_darhoA   derivative of density w.r.t. alpha*rho*A
 * @param[out] drho_dalpha   derivative of density w.r.t. alpha
 */
void rho_from_arhoA_alpha_A(
    Real arhoA, Real alpha, Real A, Real & rho, Real & drho_darhoA, Real & drho_dalpha);

/**
 * Computes density from alpha*rho*A, alpha, and area.
 *
 * @param[in] arhoA   alpha*rho*A
 * @param[in] alpha   volume fraction
 * @param[in] A       area
 * @returns density
 */
ADReal rho_from_arhoA_alpha_A(ADReal arhoA, ADReal alpha, ADReal A);

/**
 * Computes specific volume and its derivatives from rho*A, and area.
 *
 * @param[in] rhoA        rho*A
 * @param[in] A           area
 * @param[out] dv_drhoA   derivative of specific volume w.r.t. rho*A
 */
void v_from_rhoA_A(Real rhoA, Real A, Real & v, Real & dv_drhoA);

/**
 * Computes specific volume and its derivatives from rho*A, and area.
 *
 * @param[in] rhoA   rho*A
 * @param[in] A      area
 * @returns specific volume
 */
ADReal v_from_rhoA_A(ADReal rhoA, ADReal A);

/**
 * Computes specific volume and its derivatives from alpha*rho*A, volume fraction, and area.
 *
 * @param[in] arhoA        alpha*rho*A
 * @param[in] alpha        volume fraction
 * @param[in] A            area
 * @param[out] dv_darhoA   derivative of specific volume w.r.t. alpha*rho*A
 * @param[out] dv_dalpha   derivative of specific volume w.r.t. volume fraction
 */
void
v_from_arhoA_alpha_A(Real arhoA, Real alpha, Real A, Real & v, Real & dv_darhoA, Real & dv_dalpha);

/**
 * Computes specific volume and its derivatives from alpha*rho*A, volume fraction, and area.
 *
 * @param[in] arhoA        alpha*rho*A
 * @param[in] alpha        volume fraction
 * @param[in] A            area
 * @returns specific volume
 */
ADReal v_from_arhoA_alpha_A(ADReal arhoA, ADReal alpha, ADReal A);

/**
 * Computes specific volume and its derivative with respect to density
 *
 * @param[in] rho       density
 * @param[in] v         specific volume
 * @param[in] dv_drho   derivative of specific volume w.r.t. density
 */
void v_from_rho(Real rho, Real & v, Real & dv_drho);

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
 * Computes specific internal energy and its derivatives from alpha*rho*A, alpha*rho*u*A, and
 * alpha*rho*E*A
 *
 * @param[in] arhoA         alpha*rho*A
 * @param[in] arhouA        alpha*rho*u*A
 * @param[in] arhoEA        alpha*rho*E*A
 * @param[out] e            specific internal energy
 * @param[out] de_darhoA    derivative of specific internal energy w.r.t. alpha*rho*A
 * @param[out] de_darhouA   derivative of specific internal energy w.r.t. alpha*rho*u*A
 * @param[out] de_darhoEA   derivative of specific internal energy w.r.t. alpha*rho*E*A
 */
void e_from_arhoA_arhouA_arhoEA(Real arhoA,
                                Real arhouA,
                                Real arhoEA,
                                Real & e,
                                Real & de_darhoA,
                                Real & de_darhouA,
                                Real & de_darhoEA);

ADReal e_from_arhoA_arhouA_arhoEA(ADReal arhoA, ADReal arhouA, ADReal arhoEA);

/**
 * Computes specific internal energy and its derivatives from specific total energy and velocity
 *
 * @param[in] E          specific total energy
 * @param[in] vel        velocity
 * @param[out] e         specific internal energy
 * @param[out] de_dE     derivative of specific internal energy w.r.t. specific total energy
 * @param[out] de_dvel   derivative of specific internal energy w.r.t. velocity
 */
void e_from_E_vel(Real E, Real vel, Real & e, Real & de_dE, Real & de_dvel);

/**
 * Computes specific internal energy from specific total energy and velocity
 *
 * @param[in] E          specific total energy
 * @param[in] vel        velocity
 * @returns specific internal energy
 */
ADReal e_from_E_vel(ADReal E, ADReal vel);

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

/**
 * Computes specific total energy and its derivatives from alpha*rho*A and alpha*rho*E*A
 *
 * @param[in] arhoA         alpha*rho*A
 * @param[in] arhoEA        alpha*rho*E*A
 * @param[out] E            specific total energy
 * @param[out] dE_darhoA    derivative of specific total energy w.r.t. alpha*rho*A
 * @param[out] dE_darhoEA   derivative of specific total energy w.r.t. alpha*rho*E*A
 */
void E_from_arhoA_arhoEA(Real arhoA, Real arhoEA, Real & E, Real & dE_darhoA, Real & dE_darhoEA);

/**
 * Computes specific total energy from alpha*rho*A and alpha*rho*E*A
 *
 * @param[in] arhoA         alpha*rho*A
 * @param[in] arhoEA        alpha*rho*E*A
 * @returns specific total energy
 */
ADReal E_from_arhoA_arhoEA(ADReal arhoA, ADReal arhoEA);

/**
 * Computes specific total energy and its derivatives from specific internal energy and velocity
 *
 * @param[in] e          specific internal energy
 * @param[in] vel        velocity
 * @param[out] E         specific total energy
 * @param[out] dE_de     derivative of specific total energy w.r.t. specific internal energy
 * @param[out] dE_dvel   derivative of specific total energy w.r.t. velocity
 */
void E_from_e_vel(Real e, Real vel, Real & E, Real & dE_de, Real & dE_dvel);

/**
 * Computes specific enthalpy and its derivatives from specific internal energy, pressure, and
 * density
 *
 * @param[in] e          specific internal energy
 * @param[in] p          pressure
 * @param[in] rho        density
 * @param[out] h         specific enthalpy
 * @param[out] dh_de     derivative of specific enthalpy w.r.t. specific internal energy
 * @param[out] dh_dp     derivative of specific enthalpy w.r.t. pressure
 * @param[out] dh_drho   derivative of specific enthalpy w.r.t. density
 */
void h_from_e_p_rho(Real e, Real p, Real rho, Real & h, Real & dh_de, Real & dh_dp, Real & dh_drho);

ADReal h_from_e_p_rho(ADReal e, ADReal p, ADReal rho);

/**
 * Determine if inlet boundary condition should be applied
 *
 * @return true if the flow conditions are inlet, false otherwise
 * @param vel Velocity of the phase
 * @param normal Outward normal vector
 */
bool isInlet(Real vel, Real normal);
bool isInlet(ADReal vel, Real normal);

/**
 * Determine if outlet boundary condition should be applied
 *
 * @return true if the flow conditions are outlet, false otherwise
 * @param vel Velocity of the phase
 * @param normal Outward normal vector
 */
bool isOutlet(Real vel, Real normal);
bool isOutlet(ADReal vel, Real normal);
}
