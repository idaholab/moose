//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseTypes.h"
#include "PhysicalConstants.h"
#include "libmesh/libmesh_common.h"

#include <cmath>
#include <string>

/**
 * Transport properties of dilute gases: Lennard-Jones parameters of the NASA 7-coefficient
 * species, collision integrals, Chapman-Enskog viscosity and binary diffusion coefficients, the
 * modified Eucken thermal conductivity, and the low-density viscosity and thermal conductivity of
 * steam. The functions are templated so that they accept automatic differentiation types.
 */
namespace GasTransport
{
/// Parameters of the Lennard-Jones (12-6) potential of a species
struct LennardJonesParameters
{
  /// Collision diameter [m]
  Real sigma;
  /// Depth of the potential well divided by the Boltzmann constant [K]
  Real epsilon_over_k;
};

/**
 * Lennard-Jones parameters of a built-in species
 *
 * @param[in] name   species name: H2, O2, H2O, CH4, CO, CO2, or N2
 */
const LennardJonesParameters & lennardJonesParameters(const std::string & name);

/**
 * Reduced collision integral Omega(1,1)* of the Lennard-Jones potential, which enters the binary
 * diffusion coefficient, from the fit of Neufeld et al. (1972), valid for reduced temperatures
 * from 0.3 to 100
 */
template <typename CppType>
CppType
collisionIntegral11(const CppType & T_reduced)
{
  using std::exp, std::pow;
  return 1.06036 / pow(T_reduced, 0.15610) + 0.19300 * exp(-0.47635 * T_reduced) +
         1.03587 * exp(-1.52996 * T_reduced) + 1.76474 * exp(-3.89411 * T_reduced);
}

/**
 * Reduced collision integral Omega(2,2)* of the Lennard-Jones potential, which enters the
 * viscosity, from the fit of Neufeld et al. (1972), valid for reduced temperatures from 0.3 to 100
 */
template <typename CppType>
CppType
collisionIntegral22(const CppType & T_reduced)
{
  using std::exp, std::pow;
  return 1.16145 / pow(T_reduced, 0.14874) + 0.52487 * exp(-0.77320 * T_reduced) +
         2.16178 * exp(-2.43787 * T_reduced);
}

/**
 * Chapman-Enskog viscosity of a dilute gas [Pa s]
 *
 * @param[in] T            temperature [K]
 * @param[in] molar_mass   molar mass [kg/mol]
 * @param[in] lj           Lennard-Jones parameters
 */
template <typename CppType>
CppType
viscosity(const CppType & T, const Real molar_mass, const LennardJonesParameters & lj)
{
  using std::sqrt;
  const Real molecular_mass = molar_mass / PhysicalConstants::avogadro_number;
  return 5.0 / 16.0 *
         sqrt(molecular_mass * PhysicalConstants::boltzmann_constant * T / libMesh::pi) /
         (lj.sigma * lj.sigma * collisionIntegral22(T / lj.epsilon_over_k));
}

/**
 * Chapman-Enskog binary diffusion coefficient of a dilute gas pair [m^2/s], with the
 * Lorentz-Berthelot combining rules for the Lennard-Jones parameters of the pair
 *
 * @param[in] p              pressure [Pa]
 * @param[in] T              temperature [K]
 * @param[in] molar_mass_i   molar mass of the first species [kg/mol]
 * @param[in] molar_mass_j   molar mass of the second species [kg/mol]
 * @param[in] lj_i           Lennard-Jones parameters of the first species
 * @param[in] lj_j           Lennard-Jones parameters of the second species
 */
template <typename CppType>
CppType
binaryDiffusionCoefficient(const CppType & p,
                           const CppType & T,
                           const Real molar_mass_i,
                           const Real molar_mass_j,
                           const LennardJonesParameters & lj_i,
                           const LennardJonesParameters & lj_j)
{
  using std::sqrt;
  const Real sigma = 0.5 * (lj_i.sigma + lj_j.sigma);
  const Real epsilon_over_k = std::sqrt(lj_i.epsilon_over_k * lj_j.epsilon_over_k);
  const Real reduced_mass = molar_mass_i * molar_mass_j / (molar_mass_i + molar_mass_j) /
                            PhysicalConstants::avogadro_number;
  const Real k_B = PhysicalConstants::boltzmann_constant;
  return 3.0 / 16.0 * sqrt(2 * libMesh::pi * k_B * T / reduced_mass) * k_B * T /
         (p * libMesh::pi * sigma * sigma * collisionIntegral11(T / epsilon_over_k));
}

/**
 * Thermal conductivity of a dilute gas from the modified Eucken correlation [W/(m K)],
 * k = mu R_s (1.32 cp/R + 0.45), with the specific gas constant R_s
 *
 * @param[in] mu           viscosity [Pa s]
 * @param[in] cp_over_R    molar isobaric heat capacity divided by the gas constant
 * @param[in] R_specific   specific gas constant [J/(kg K)]
 */
template <typename CppType>
CppType
modifiedEuckenConductivity(const CppType & mu, const CppType & cp_over_R, const Real R_specific)
{
  return mu * R_specific * (1.32 * cp_over_R + 0.45);
}

/**
 * Viscosity of steam in the limit of zero density [Pa s], from the IAPWS 2008 formulation, as in
 * Water97FluidProperties
 */
template <typename CppType>
CppType
steamViscosityZeroDensity(const CppType & T)
{
  using std::sqrt;
  // Critical temperature of water [K]
  const CppType Tbar = T / 647.096;
  const CppType sum =
      1.67752 + 2.20462 / Tbar + 0.6366564 / (Tbar * Tbar) - 0.241605 / (Tbar * Tbar * Tbar);
  // The formulation gives the viscosity in micropascal seconds
  return 1e-6 * 100 * sqrt(Tbar) / sum;
}

/**
 * Thermal conductivity of steam in the limit of zero density [W/(m K)], from the IAPWS 2011
 * formulation. Water97FluidProperties uses the IAPS 1985 formulation, whose zero-density term
 * agrees within 2% below 1500 K but decreases above about 2200 K and becomes negative near 3300 K.
 */
template <typename CppType>
CppType
steamConductivityZeroDensity(const CppType & T)
{
  using std::sqrt;
  // Critical temperature of water [K]
  const CppType Tbar = T / 647.096;
  const CppType sum =
      2.443221e-3 +
      (1.323095e-2 + (6.770357e-3 + (-3.454586e-3 + 4.096266e-4 / Tbar) / Tbar) / Tbar) / Tbar;
  // The formulation gives the conductivity in milliwatts per meter and kelvin
  return 1e-3 * sqrt(Tbar) / sum;
}
}
