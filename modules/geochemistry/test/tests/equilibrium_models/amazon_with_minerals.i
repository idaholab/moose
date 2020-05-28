[EquilibriumReactionSolver]
  model_definition = definition
  swap_out_of_basis = "Al+++ Fe++"
  swap_into_basis = "Kaolinite Hematite"
  charge_balance_species = "Cl-"
# Approximately TDS = 44mg/kg
# Note that TDS = (mass_non-water) / (mass_solvent_water + mass_non-water),
# so with mass_solvent_water = 1kg, mass_non-water = 4.4E-5kg kg, and total_mass = 1.000044kg
# concentration of SiO2(aq) = 7mg/kg -> moles = 7E-3 * 1.000044 / 60.0843 = 0.0001165
# concentration of Cl- = 1.9mg/kg -> moles = 1.9E-3 * 1.000044 / 35.4530 = 5.359E-5
# concentration of Ca++ = 4.3mg/kg -> moles = 4.3E-3 * 1.000044 / 40.0800 = 1.073E-4
# etc
# kaolinite 1cm^3 = 0.01 mol
# hematite 1cm^3 = 0.033 mol
#
# HOWEVER, Bethke reports an equilibrium Cl- molality of 1.38E-4mol/kg(solvent water) (see Table 6.7).  This means that the bulk composition of Cl- must be at least 1.38E-4mol since the stoichiometric coefficients of Cl- in all secondary species are positive.  This implies the bulk concentration of Cl- must be more than 1.9mg/kg.  There is probably a typo in the book: the bulk concentration is presumably 4.9mg/kg, leading to moles = 1.383E-4
#
  constraint_species = "H2O              H+                   O2(aq)        SiO2(aq)           Kaolinite                  Hematite                   Ca++               Mg++               Na+                HCO3-              SO4--              Cl-      "
  constraint_value = "  1.0              3.162E-7             1.813E-4      0.0001165          1E-2                       0.033                      0.0001073          4.526E-5           7.83E-5            0.0003114          3.1233E-5          1.383E-4"
  constraint_meaning = "kg_solvent_water activity             free_molality moles_bulk_species free_moles_mineral_species free_moles_mineral_species moles_bulk_species moles_bulk_species moles_bulk_species moles_bulk_species moles_bulk_species moles_bulk_species"
  prevent_precipitation = "Nontronit-Ca Nontronit-Mg Nontronit-Na Hematite Kaolinite Beidellit-Ca Beidellit-H Beidellit-Mg Beidellit-Na Pyrophyllite Gibbsite Paragonite Quartz"
[]

[UserObjects]
  [./definition]
    type = GeochemicalModelDefinition
    database_file = "../../../database/moose_geochemdb.json"
    basis_species = "H2O H+ SiO2(aq) Al+++ Fe++ Ca++ Mg++ Na+ HCO3- SO4-- Cl- O2(aq)"
    equilibrium_minerals = "Nontronit-Ca Nontronit-Mg Nontronit-Na Hematite Kaolinite Beidellit-Ca Beidellit-H Beidellit-Mg Beidellit-Na Pyrophyllite Gibbsite Paragonite Quartz"
  [../]
[]
