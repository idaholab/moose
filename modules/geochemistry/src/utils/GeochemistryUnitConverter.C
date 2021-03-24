//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GeochemistryUnitConverter.h"
#include "GeochemistryConstants.h"

namespace GeochemistryUnitConverter
{

Real
fromMoles(Real moles,
          const GeochemistryUnits & unit,
          const std::string & species_name,
          const ModelGeochemicalDatabase & mgd)
{
  Real factor = 0.0; // return moles * factor
  if (mgd.basis_species_index.count(species_name) != 0)
    factor = conversionFactor(unit,
                              mgd.basis_species_index.at(species_name),
                              species_name,
                              mgd.basis_species_molecular_weight,
                              mgd.basis_species_molecular_volume,
                              mgd.basis_species_mineral);
  else if (mgd.eqm_species_index.count(species_name) != 0)
    factor = conversionFactor(unit,
                              mgd.eqm_species_index.at(species_name),
                              species_name,
                              mgd.eqm_species_molecular_weight,
                              mgd.eqm_species_molecular_volume,
                              mgd.eqm_species_mineral);
  else if (mgd.kin_species_index.count(species_name) != 0)
    factor = conversionFactor(unit,
                              mgd.kin_species_index.at(species_name),
                              species_name,
                              mgd.kin_species_molecular_weight,
                              mgd.kin_species_molecular_volume,
                              mgd.kin_species_mineral);
  else
    mooseError("GeochemistryUnitConverter: ",
               species_name,
               " is not a basis, equilibrium or kinetic species");
  return moles * factor;
}

Real
toMoles(Real quantity,
        const GeochemistryUnits & unit,
        const std::string & species_name,
        const ModelGeochemicalDatabase & mgd)
{
  return quantity / fromMoles(1.0, unit, species_name, mgd);
}

Real
conversionFactor(const GeochemistryUnits & unit,
                 unsigned ind,
                 const std::string & name,
                 const std::vector<Real> & mol_weight,
                 const std::vector<Real> & mol_volume,
                 const std::vector<bool> & is_mineral)
{
  Real factor = 1.0;
  switch (unit)
  {
    case GeochemistryUnits::DIMENSIONLESS:
    case GeochemistryUnits::MOLES:
    case GeochemistryUnits::MOLAL:
      factor = 1.0;
      break;
    case GeochemistryUnits::KG:
    case GeochemistryUnits::KG_PER_KG_SOLVENT:
      factor = 1E-3 * mol_weight[ind];
      break;
    case GeochemistryUnits::G:
    case GeochemistryUnits::G_PER_KG_SOLVENT:
      factor = mol_weight[ind];
      break;
    case GeochemistryUnits::MG:
    case GeochemistryUnits::MG_PER_KG_SOLVENT:
      factor = 1.0E3 * mol_weight[ind];
      break;
    case GeochemistryUnits::UG:
    case GeochemistryUnits::UG_PER_KG_SOLVENT:
      factor = 1.0E6 * mol_weight[ind];
      break;
    case GeochemistryUnits::CM3:
      if (!is_mineral[ind])
        mooseError("GeochemistryUnitConverter: Cannot use CM3 units for species ",
                   name,
                   " because it is not a mineral");
      factor = mol_volume[ind];
      break;
  }
  return factor;
}
} // namespace GeochemistryUnitConverter
