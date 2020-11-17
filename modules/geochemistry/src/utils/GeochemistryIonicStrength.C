//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GeochemistryIonicStrength.h"
#include "libmesh/utility.h"

GeochemistryIonicStrength::GeochemistryIonicStrength(Real max_ionic_strength,
                                                     Real max_stoichiometric_ionic_strength,
                                                     bool use_only_basis_molality,
                                                     bool use_only_Cl_molality)
  : _max_ionic_strength(max_ionic_strength),
    _max_stoichiometric_ionic_strength(max_stoichiometric_ionic_strength),
    _use_only_basis_molality(use_only_basis_molality),
    _use_only_Cl_molality(use_only_Cl_molality)
{
}

Real
GeochemistryIonicStrength::ionicStrength(const ModelGeochemicalDatabase & mgd,
                                         const std::vector<Real> & basis_species_molality,
                                         const std::vector<Real> & eqm_species_molality,
                                         const std::vector<Real> & kin_species_molality) const
{
  const unsigned num_basis = mgd.basis_species_charge.size();
  const unsigned num_eqm = mgd.eqm_species_charge.size();
  const unsigned num_kin = mgd.kin_species_charge.size();
  if (num_basis != basis_species_molality.size())
    mooseError("Ionic strength calculation: Number of basis species in mgd not equal to the size "
               "of basis_species_molality");
  if (num_eqm != eqm_species_molality.size())
    mooseError("Ionic strength calculation: Number of equilibrium species in mgd not equal to the "
               "size of eqm_species_molality");
  if (num_kin != kin_species_molality.size())
    mooseError("Ionic strength calculation: Number of kinetic species in mgd not equal to the size "
               "of kin_species_molality");

  Real ionic_strength = 0.0;
  for (unsigned i = 0; i < num_basis; ++i)
    ionic_strength += Utility::pow<2>(mgd.basis_species_charge[i]) * basis_species_molality[i];
  if (!_use_only_basis_molality)
  {
    for (unsigned i = 0; i < num_eqm; ++i)
      if (!mgd.surface_sorption_related[i])
        ionic_strength += Utility::pow<2>(mgd.eqm_species_charge[i]) * eqm_species_molality[i];
    for (unsigned i = 0; i < num_kin; ++i)
      ionic_strength += Utility::pow<2>(mgd.kin_species_charge[i]) * kin_species_molality[i] /
                        basis_species_molality[0]; // kin_species_molality is actually the number of
                                                   // moles of the kinetic species
  }

  return std::max(0.0, std::min(_max_ionic_strength, 0.5 * ionic_strength));
}

Real
GeochemistryIonicStrength::stoichiometricIonicStrength(
    const ModelGeochemicalDatabase & mgd,
    const std::vector<Real> & basis_species_molality,
    const std::vector<Real> & eqm_species_molality,
    const std::vector<Real> & kin_species_molality) const
{
  const unsigned num_basis = mgd.basis_species_charge.size();
  const unsigned num_eqm = mgd.eqm_species_charge.size();
  const unsigned num_kin = mgd.kin_species_charge.size();
  if (num_basis != basis_species_molality.size())
    mooseError("Stoichiometric ionic strength calculation: Number of basis species in mgd not "
               "equal to the size of basis_species_molality");
  if (num_eqm != eqm_species_molality.size())
    mooseError("Stoichiometric ionic strength calculation: Number of equilibrium species in mgd "
               "not equal to the size of eqm_species_molality");
  if (num_kin != kin_species_molality.size())
    mooseError("Stoichiometric ionic strength calculation: Number of kinetic species in mgd not "
               "equal to the size of kin_species_molality");

  Real ionic_strength = 0.0;
  if (_use_only_Cl_molality)
  {
    if (mgd.basis_species_index.count("Cl-"))
      ionic_strength = basis_species_molality[mgd.basis_species_index.at("Cl-")];
    else if (mgd.eqm_species_index.count("Cl-"))
      ionic_strength = eqm_species_molality[mgd.eqm_species_index.at("Cl-")];
    else if (mgd.kin_species_index.count("Cl-"))
      ionic_strength = kin_species_molality[mgd.kin_species_index.at("Cl-")];
    else
      mooseError("GeochemistryIonicStrength: attempting to compute stoichiometric ionic strength "
                 "using only the Cl- molality, but Cl- does not appear in the geochemical system");
    return std::max(0.0, std::min(_max_stoichiometric_ionic_strength, ionic_strength));
  }

  for (unsigned i = 0; i < num_basis; ++i)
    ionic_strength += Utility::pow<2>(mgd.basis_species_charge[i]) * basis_species_molality[i];
  if (!_use_only_basis_molality)
  {
    for (unsigned i = 0; i < num_eqm; ++i)
      if (!mgd.surface_sorption_related[i])
      {

        if (mgd.eqm_species_charge[i] != 0.0)
          ionic_strength += Utility::pow<2>(mgd.eqm_species_charge[i]) * eqm_species_molality[i];
        else
        {
          for (unsigned j = 0; j < num_basis; ++j)
            ionic_strength += Utility::pow<2>(mgd.basis_species_charge[j]) *
                              eqm_species_molality[i] * mgd.eqm_stoichiometry(i, j);
        }
      }
    for (unsigned i = 0; i < num_kin;
         ++i) // kin_species_molality is actually the number of moles, not molality
      if (mgd.kin_species_charge[i] != 0.0)
        ionic_strength += Utility::pow<2>(mgd.kin_species_charge[i]) * kin_species_molality[i] /
                          basis_species_molality[0];
  }

  return std::max(0.0, std::min(_max_stoichiometric_ionic_strength, 0.5 * ionic_strength));
}

void
GeochemistryIonicStrength::setMaxIonicStrength(Real max_ionic_strength)
{
  _max_ionic_strength = max_ionic_strength;
}

Real
GeochemistryIonicStrength::getMaxIonicStrength() const
{
  return _max_ionic_strength;
}

void
GeochemistryIonicStrength::setMaxStoichiometricIonicStrength(Real max_stoichiometric_ionic_strength)
{
  _max_stoichiometric_ionic_strength = max_stoichiometric_ionic_strength;
}

Real
GeochemistryIonicStrength::getMaxStoichiometricIonicStrength() const
{
  return _max_stoichiometric_ionic_strength;
}

void
GeochemistryIonicStrength::setUseOnlyBasisMolality(bool use_only_basis_molality)
{
  _use_only_basis_molality = use_only_basis_molality;
}

Real
GeochemistryIonicStrength::getUseOnlyBasisMolality() const
{
  return _use_only_basis_molality;
}

void
GeochemistryIonicStrength::setUseOnlyClMolality(bool use_only_Cl_molality)
{
  _use_only_Cl_molality = use_only_Cl_molality;
}

Real
GeochemistryIonicStrength::getUseOnlyClMolality() const
{
  return _use_only_Cl_molality;
}
