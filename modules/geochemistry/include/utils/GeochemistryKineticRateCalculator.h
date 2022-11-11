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
 * This controls the direction of a kinetic rate
 * BOTH: both dissolution and precipitation are allowed
 * PRECIPITATION: if log10(activity_product) < log10(equilibrium_constant) so that dissolution
 * should occur, then the rate is set to zero
 * DISSOLUTION: if log10(activity_product) > log10(equilibrium_constant) so that precipitation
 * should occur, then the rate is set to zero
 * RAW: rate is unimpacted by sign of log10(activity_product) / log10(equilibrium_constant)
 * DEATH: rate is unimpacted by sign of log10(activity_product) / log10(equilibrium_constant).  This
 * is different from RAW because no reaction products are produced: the kinetic rate is *only* used
 * to modify the kinetic species' mass.  This is useful to model biological mortality.
 */
enum DirectionChoiceEnum
{
  BOTH,
  DISSOLUTION,
  PRECIPITATION,
  RAW,
  DEATH
};

/**
 * Holds a user-specified description of a kinetic rate
 *
 * @param kinetic_species name of the kinetic species
 * @param intrinsic_rate_constant Note that intrinsic_rate_constant * area_quantity *
 * [kinetic_species mass] must have dimensions mol.s^-1
 * @param area_quantity Either 1, or the fixed surface area of the kinetic species, or a specific
 * surface area (m^2/g)
 * @param multiply_by_mass whether the rate should be multiplied by the kinetic_species mass
 * @param kinetic_molal_index, rate is multiplied by kinetic_species_molality^kinetic_molal_index
 * @param kinetic_monod_index, rate is multiplied by 1.0 /
 * (kinetic_species_molality^kinetic_molal_index +
 * kinetic_half_saturation^kinetic_molal_index)^kinetic_monod_index
 * @param kinetic_half_saturation, rate is multiplied by 1.0 /
 * (kinetic_species_molality^kinetic_molal_index +
 * kinetic_half_saturation^kinetic_molal_index)^kinetic_monod_index
 * @param promoting_species names of species (which must be primary or secondary species in the
 * system)
 * @param promoting_indices indices of mass, fugacity, activity or mobility (as appropriate)
 * @param promoting_monod_indices monod indices of mass, fugacity, activity or mobility (as
 * appropriate)
 * @param promoting_half_saturation half saturation values of all promoting species
 * @param theta exponent of (Q/K)
 * @param eta exponent of |1-(Q/K)^theta|
 * @param activation_energy in J.mol^-1
 * @param one_over_T0 measured in 1/Kelvin
 * @param direction: whether this kinetic rate is designed for: "both" precipitation and
 * dissolution; "precipitation" only; "dissolution" only; and "raw" or "death" mean the rate does
 * not depend on the sign of 1-(Q/K)
 * @param progeny: A non-kinetic species that catalyses the reaction, and potentially gets produced
 * or consumed by it
 * @param progeny_efficiency: When one mole of reaction is catalysed, progeny_efficiency moles of
 * the progeny is created
 * @param kinetic_bio_efficiency: the efficiency of a biologically-catalysed reaction, that is, when
 * one mole of reaction is catalysed, the biomass increases by kinetic_bio_efficiency moles
 * @param energy captured: energy captured by a biologically-catalysed reaction, essentially this
 * reduces the equilibrium constant of the reaction by exp(-energy_capture / R / Tk)
 */
struct KineticRateUserDescription
{
  KineticRateUserDescription(const std::string & kinetic_species_name,
                             Real intrinsic_rate_constant,
                             Real area_quantity,
                             bool multiply_by_mass,
                             Real kinetic_molal_index,
                             Real kinetic_monod_index,
                             Real kinetic_half_saturation,
                             const std::vector<std::string> & promoting_species,
                             const std::vector<Real> & promoting_indices,
                             const std::vector<Real> & promoting_monod_indices,
                             const std::vector<Real> & promoting_half_saturation,
                             Real theta,
                             Real eta,
                             Real activation_energy,
                             Real one_over_T0,
                             DirectionChoiceEnum direction,
                             std::string progeny,
                             Real progeny_efficiency,
                             Real kinetic_bio_efficiency,
                             Real energy_captured)
    : kinetic_species_name(kinetic_species_name),
      intrinsic_rate_constant(intrinsic_rate_constant),
      area_quantity(area_quantity),
      multiply_by_mass(multiply_by_mass),
      kinetic_molal_index(kinetic_molal_index),
      kinetic_monod_index(kinetic_monod_index),
      kinetic_half_saturation(kinetic_half_saturation),
      promoting_species(promoting_species),
      promoting_indices(promoting_indices),
      promoting_monod_indices(promoting_monod_indices),
      promoting_half_saturation(promoting_half_saturation),
      theta(theta),
      eta(eta),
      activation_energy(activation_energy),
      one_over_T0(one_over_T0),
      direction(direction),
      progeny(progeny),
      progeny_efficiency(progeny_efficiency),
      kinetic_bio_efficiency(kinetic_bio_efficiency),
      energy_captured(energy_captured)
  {
    if (promoting_species.size() != promoting_indices.size())
      mooseError("The promoting_species and promoting_indices vectors must be the same size");
    if (promoting_species.size() != promoting_monod_indices.size())
      mooseError("The promoting_species and promoting_monod_indices vectors must be the same size");
    if (promoting_species.size() != promoting_half_saturation.size())
      mooseError(
          "The promoting_species and promoting_half_saturation vectors must be the same size");
    std::unordered_map<std::string, int> check_for_repeats;
    for (const std::string & name : promoting_species)
      if (check_for_repeats.count(name) == 1)
        mooseError("Promoting species ", name, " has already been provided with an exponent");
      else
        check_for_repeats[name] = 1;
  };

  bool operator==(const KineticRateUserDescription & rhs) const
  {
    return (kinetic_species_name == rhs.kinetic_species_name) &&
           (intrinsic_rate_constant == rhs.intrinsic_rate_constant) &&
           (area_quantity == rhs.area_quantity) && (multiply_by_mass == rhs.multiply_by_mass) &&
           (kinetic_molal_index == rhs.kinetic_molal_index) &&
           (kinetic_monod_index == rhs.kinetic_monod_index) &&
           (kinetic_half_saturation == rhs.kinetic_half_saturation) &&
           (promoting_species == rhs.promoting_species) &&
           (promoting_indices == rhs.promoting_indices) &&
           (promoting_monod_indices == rhs.promoting_monod_indices) &&
           (promoting_half_saturation == rhs.promoting_half_saturation) && (theta == rhs.theta) &&
           (eta == rhs.eta) && (activation_energy == rhs.activation_energy) &&
           (one_over_T0 == rhs.one_over_T0) && (direction == rhs.direction) &&
           (progeny == rhs.progeny) && (progeny_efficiency == rhs.progeny_efficiency) &&
           (kinetic_bio_efficiency == rhs.kinetic_bio_efficiency) &&
           (energy_captured == rhs.energy_captured);
  };

  std::string kinetic_species_name;
  Real intrinsic_rate_constant;
  Real area_quantity;
  bool multiply_by_mass;
  Real kinetic_molal_index;
  Real kinetic_monod_index;
  Real kinetic_half_saturation;
  std::vector<std::string> promoting_species;
  std::vector<Real> promoting_indices;
  std::vector<Real> promoting_monod_indices;
  std::vector<Real> promoting_half_saturation;
  Real theta;
  Real eta;
  Real activation_energy;
  Real one_over_T0;
  DirectionChoiceEnum direction;
  std::string progeny;
  Real progeny_efficiency;
  Real kinetic_bio_efficiency;
  Real energy_captured;
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
 * (molality of kinetic_species)^kinetic_molal_index
 * 1 / ((molality of kinetic_species)^kinetic_molal_index +
 * kinetic_half_saturation^kinetic_molal_index)^kinetic_monod_index
 * product over the promoting_species of m^(promoting_index) / (m^(promoting_index) +
 * half_saturation^(promoting_index))^(promoting_monod_index)
 * |1 - (Q/K)^theta|^eta, where K = eqm_constant_in_database * exp(- energy_captured / R / TK)
 * exp(activation_energy / R * (1/T0 - 1/T)) D(1 - (Q/K))
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
 * K is the reaction's equilibrium constant (defined in the database file) multiplied by
 * exp(-energy_captured / (RT)).
 * R = 8.314472 m^2.kg.s^-2.K^-1.mol^-1 = 8.314472 J.K^-1.mol^-1 is
 * the gas constant. T is the temperature in Kelvin. T0 is a reference temperature, in Kelvin.  It
 * is inputted as 1/T0 so that 1/T0 = 0 is possible.
 *
 * D(x) depends on direction.  If direction == BOTH then D(x) = sgn(x).  If direction == DISSOLUTION
 * then D(x) = (x>0)?1:0.  If direction == PRECIPITATION then D(x) = (x<0)?-1:0.  If direction ==
 * RAW then D=1.  If direction == DEATH then D=1.
 *
 * The amount of kinetic species that is generated is kinetic_bio_efficiency * rate.  Note that rate
 * > 0 for dissolution of the kinetic species, so kinetic_bio_efficiency defaults to -1.  However,
 * for biogeochemistry, it is appropriate to set kinetic_bio_efficiency > 0, so that "dissolution"
 * of the biomass (with rate > 0) generates further biomass
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
 * D(1 - (Q/K))
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
                   const std::vector<Real> & promoting_monod_indices,
                   const std::vector<Real> & promoting_half_saturation,
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
