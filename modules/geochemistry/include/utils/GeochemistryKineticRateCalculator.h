//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseTypes.h"
#include "DenseMatrix.h"
#include "GeochemistryConstants.h"

/**
 * Holds a user-specified description of a kinetic rate
 *
 * @param kinetic_species name of the kinetic species
 * @param intrinsic_rate_constant Note that intrinsic_rate_constant * area_quantity *
 * [kinetic_species mass] must have dimensions mol.s^-1
 * @param area_quantity Either 1, or the fixed surface area of the kinetic species, or a specific
 * surface area (m^2/g)
 * @param multiply_by_mass whether the rate should be multiplied by the kinetic_species mass
 * @param promoting_species names of species (which must be primary or secondary species in the
 * system)
 * @param promoting_indices indices of mass, fugacity, activity or mobility (as appropriate)
 * @param theta exponent of (Q/K)
 * @param eta exponent of |1-(Q/K)^theta|
 * @param activation_energy in J.mol^-1
 * @param one_over_T0 measured in 1/Kelvin
 */
struct KineticRateUserDescription
{
  KineticRateUserDescription(const std::string & kinetic_species_name,
                             Real intrinsic_rate_constant,
                             Real area_quantity,
                             bool multiply_by_mass,
                             const std::vector<std::string> & promoting_species,
                             const std::vector<Real> & promoting_indices,
                             Real theta,
                             Real eta,
                             Real activation_energy,
                             Real one_over_T0)
    : kinetic_species_name(kinetic_species_name),
      intrinsic_rate_constant(intrinsic_rate_constant),
      area_quantity(area_quantity),
      multiply_by_mass(multiply_by_mass),
      promoting_species(promoting_species),
      promoting_indices(promoting_indices),
      theta(theta),
      eta(eta),
      activation_energy(activation_energy),
      one_over_T0(one_over_T0)
  {
    if (promoting_species.size() != promoting_indices.size())
      mooseError("The promoting_species and promoting_indices vectors must be the same size");
    std::unordered_map<std::string, int> check_for_repeats;
    for (const std::string & name : promoting_species)
      if (check_for_repeats.count(name) == 1)
        mooseError("Promoting species ", name, " has already been provided with an exponent");
      else
        check_for_repeats[name] = 1;
  };

  std::string kinetic_species_name;
  Real intrinsic_rate_constant;
  Real area_quantity;
  bool multiply_by_mass;
  std::vector<std::string> promoting_species;
  std::vector<Real> promoting_indices;
  Real theta;
  Real eta;
  Real activation_energy;
  Real one_over_T0;
};

/**
 * Provides a parametric description of a general kinetic rate.  This is a separate class with very
 * little functionality partly so that it can be used by multiple other classes
 * (PertinentGeochemicalSystem, GeochemistryKineticRate) but also so that it can be easily added to
 * if more general kinetic rates are ever required.
 *
 * The rate is a product of the following terms:
 *
 * intrinsic_rate_constant
 * area_quantity
 * mass of the kinetic_species, measured in grams, if multiply_by_mass is true
 * product over the promoting_species of m^(promoting_index)
 * |1 - (Q/K)^theta|^eta
 * exp(activation_energy / R * (1/T0 - 1/T))
 * sign(1 - (Q/K))
 *
 * Some explanation may be useful:
 *
 * intrinsic_rate_constant * area_quantity * [mass of kinetic_species] has units mol.s^-1.  This
 * allows for the following common possibilities:
 * - intrinsic_rate_constant is measured in mol.s^-1.  In this case, the user should set
 * area_quantity=1 and multiply_by_mass=false
 * - intrinsic_rate_constant is measured in mol.s^-1/kg(solvent_water).  In this case the user
 * should set area_quantity=1 and multipliy_by_mass=false, but ensure that
 * promoting_species={"H2O",...} and promoting_index={1,...}.
 * - intrinsic_rate_constant is measured in mol.s^-1/area_of_kinetic_mineral, and the area of the
 * kinetic mineral is fixed.  In this case the user should set
 * area_quantity=area_of_kinetic_mineral and multiply_by_mass=false.
 * - intrinsic_rate_constant is measured in mol.s^-1/area_of_kinetic_mineral, and the area of the
 * kinetic mineral depends on its mass.  In this case, the user should set
 * area_quantity=specific_area_of_mineral (in m^2/g) and multiply_by_mass=true
 *
 * The "m" in the promoting species product is:
 * - mass of solvent water (in kg) if promoting_species="H2O"
 * - fugacity of a gas if promoting_species is a gas
 * - activity if promoting_species is either "H+" or "OH-"
 * - mobility, otherwise
 *
 * Q is the activity product, defined by the kinetic_species reaction (defined in the database
 * file).
 * K is the reaction's equilibrium constant (defined in the database file).
 * R = 8.314472 m^2.kg.s^-2.K^-1.mol^-1 = 8.314472 J.K^-1.mol^-1 is the gas constant.
 * T is the temperature in Kelvin.
 * T0 is a reference temperature, in Kelvin.  It is inputted as 1/T0 so that 1/T0 = 0 is possible.
 */
namespace GeochemistryKineticRateCalculator
{
/**
 * Calclates a kinetic rate and its derivative. The rate is a product of the following terms:
 *
 * intrinsic_rate_constant
 * area_quantity
 * mass of the kinetic_species, measured in grams, if multiply_by_mass is true
 * product over the promoting_species of m^(promoting_index)
 * |1 - (Q/K)^theta|^eta
 * exp(activation_energy / R * (1/T0 - 1/T))
 * sign(1 - (Q/K))
 *
 * @param promoting_indices of the basis species and equilibrium species.  Note that this is
 * different from description.promoting_indices (which is paired with description.promoting_species)
 * because it is the promoting indices for the current basis and equilibrium.  For computational
 * efficiency promoting_indices[0:num_basis-1] are the promoting indices for the current basis
 * species while promoting_indices[num_basis:] are the promoting indices for the current eqm species
 * @param description contains definition of intrinsic_rate_constant, area_quantity, etc
 * @param basis_species_name vector of basis species names
 * @param basis_species_gas i^th element is true if the i^th basis species is a gas
 * @param basis_molality vector of the basis molalities (zeroth element is kg of solvent water, and
 * this is mole number for mineral species)
 * @param basis_activity vector of basis activities
 * @param basis_activity_known i^th element is true if the activity for the i^th basis species has
 * been constrained by the user
 * @param eqm_species_name vector of eqm species names
 * @param eqm_species_gas i^th element is true if the i^th eqm species is a gas
 * @param eqm_molality vector of the eqm molalities (zeroth element is kg of solvent water, and this
 * is mole number for mineral species)
 * @param eqm_activity vector of eqm activities
 * @param eqm_stoichiometry matrix of stoichiometric coefficient
 * @param kin_moles moles of the kinetic species
 * @param kin_species_molecular_weight molecular weight (g/mol) of the kinetic species
 * @param kin_stoichiometry kinetic stoichiometry matrix (only row=kin is used)
 * @param kin the row of kin_stoichiometry that corresponds to the kinetic species
 * @param log10K log10(equilibrium constant) of the kinetic species
 * @param log10_activity_product log10(activity product) of the kinetic species
 * @param temp_deg temperature in degC
 * @param[out] rate the rate computed by this method
 * @param[out] drate_dkin d(rate)/d(mole number of the kinetic species)
 * @param[out] drate_dmol d(rate)/d(basis molality[i])
 */
void calculateRate(const std::vector<Real> & promoting_indices,
                   const KineticRateUserDescription & description,
                   const std::vector<std::string> & basis_species_name,
                   const std::vector<bool> & basis_species_gas,
                   const std::vector<Real> & basis_molality,
                   const std::vector<Real> & basis_activity,
                   const std::vector<bool> & basis_activity_known,
                   const std::vector<std::string> & eqm_species_name,
                   const std::vector<bool> & eqm_species_gas,
                   const std::vector<Real> & eqm_molality,
                   const std::vector<Real> & eqm_activity,
                   const DenseMatrix<Real> & eqm_stoichiometry,
                   Real kin_moles,
                   Real kin_species_molecular_weight,
                   Real log10K,
                   Real log10_activity_product,
                   const DenseMatrix<Real> & kin_stoichiometry,
                   unsigned kin,
                   Real temp_degC,
                   Real & rate,
                   Real & drate_dkin,
                   std::vector<Real> & drate_dmol);
}
