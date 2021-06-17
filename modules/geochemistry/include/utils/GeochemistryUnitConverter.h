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
#include "PertinentGeochemicalSystem.h"

/**
 * Utilities to convert to and from mole units
 */
namespace GeochemistryUnitConverter
{
enum class GeochemistryUnit
{
  DIMENSIONLESS,
  MOLES,
  MOLAL,
  KG,
  G,
  MG,
  UG,
  KG_PER_KG_SOLVENT,
  G_PER_KG_SOLVENT,
  MG_PER_KG_SOLVENT,
  UG_PER_KG_SOLVENT,
  CM3
};

/**
 * Calculates the amount of "unit"s of species_name in 1 mole, OR in 1 molal, whichever is
 * appropriate.  Example: if moles = 2.0 and unit = CM3 and species_name = Halite, this function
 * will return the number of cm^3 in 1 mole of Halite.  Example: if moles = 2.0 and unit = MG_PER_KG
 * and species_name = Halite, this function will return the number of mg/kg_solvent_water in a 2.0
 * molal solution of Halite.
 * @param moles the amount of moles of species_name
 * @param unit the unit to convert to.  If unit is DIMENSIONLESS,
 * MOLES or MOLAL, this returns quantity.  If unit is KG, G, MG, UG or CM3, then "moles" is the mole
 * number.  If unit is KG_PER_KG_SOLVENT, G_PER_KG_SOLVENT, MG_PER_KG_SOLVENT, UG_PER_KG_SOLVENT,
 * then "moles" is assumed to be the molality.
 * @param species_name the name of the substance - this should appear in mgd
 * @param mgd the database corresponding to the model, which should contain information about
 * species_name
 */
Real fromMoles(Real moles,
               const GeochemistryUnit unit,
               const std::string & species_name,
               const ModelGeochemicalDatabase & mgd);

/**
 * Calculates the amount of moles corresponding to "quantity" "unit"s of species name, OR calculates
 * the molality corresponding to "quantity" "unit"s of species name, whichever is appropriate.
 * Example: if quantity = 2.0 and unit = CM3 and species_name = Halite, this function will return
 * the number of moles in 2 cm^3 of Halite.  Example: if quantity = 2.0 and unit = MG_PER_KG and
 * species_name = Halite, this function will return the molality of a 2 mg/kg_solvent_water Halite
 * solution.
 * @param quantity the amount of substance expressed in "unit" units
 * @param unit the unit of measurement of to_convert.  If unit is DIMENSIONLESS,
 * MOLES or MOLAL, this returns quantity.  If unit is KG, G, MG, UG or CM3, this function returns
 * the number of moles.  If unit is KG_PER_KG_SOLVENT, G_PER_KG_SOLVENT, MG_PER_KG_SOLVENT,
 * UG_PER_KG_SOLVENT, this function returns the molality.
 * @param species_name the name of the substance - this should appear in mgd
 * @param mgd the database corresponding to the model, which should contain information about
 * species_name
 * @return the number of moles, or the molality, whichever is appropriate
 */
Real toMoles(Real quantity,
             const GeochemistryUnit unit,
             const std::string & species_name,
             const ModelGeochemicalDatabase & mgd);

/**
 * Calculates the number of "unit" in 1 mole of substance, OR calculates the number of "unit" in 1
 * molal of substance, whichever is appropriate.  Example: if unit = G, this function will return
 * the molecular weight of the substance.  Example: if unit = CM3, this function returns the
 * molecular volume of the substance, or will produce an error if the substance is not a mineral.
 * Example: if unit = MG_PER_KG, then the molal quantity returned is 1E3 * (molecular weight of the
 * substance).  You will probably want to use toMoles or fromMoles rather than this function.
 * @param unit The unit that you wish to convert 1 mol of substance to.  If unit is DIMENSIONLESS,
 * MOLES or MOLAL, this returns 1.0.  If unit is KG, G, MG, UG or CM3, this function returns the
 * number in 1 mole.  If unit is KG_PER_KG_SOLVENT, G_PER_KG_SOLVENT, MG_PER_KG_SOLVENT,
 * UG_PER_KG_SOLVENT, this function returns the number in 1 molal.
 * @param ind The index of mol_weight and mol_volume and is_mineral corresponding to the substance
 * @param name The name of the substance
 * @param mol_weight mol_weight[ind] is the molecular weight (g/mol) of the substance
 * @param mol_volume mol_volume[ind] is the molecular volume (cm^3/mol) of the substance
 * @param is_mineral is_mineral[ind] = true iff the substance is a mineral
 * @return 1 mol of substance will be this many "unit"s of that substance
 */
Real conversionFactor(const GeochemistryUnit unit,
                      unsigned ind,
                      const std::string & name,
                      const std::vector<Real> & mol_weight,
                      const std::vector<Real> & mol_volume,
                      const std::vector<bool> & is_mineral);
}
