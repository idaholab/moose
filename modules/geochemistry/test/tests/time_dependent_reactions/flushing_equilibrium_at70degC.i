# Alkali flushing of a reservoir (an example of flushing)
# This input file finds the equilibrium bulk moles of aqueous species (Cl-, Na+, etc) for input into flushing.i.  It will have to be re-run when temperature dependence is included in geochemistry.  Note that end_time = 0.0days
[GlobalParams]
  point = '0 0 0'
[]

[TimeIndependentReactionSolver]
  model_definition = definition
  geochemistry_reactor_name = reactor
  charge_balance_species = "Cl-"
  swap_into_basis = "Calcite Dolomite-ord Muscovite Kaolinite Quartz"
  swap_out_of_basis = "HCO3- Mg++ K+ Al+++ SiO2(aq)"
  constraint_species = "H2O H+   Cl- Na+ Ca++ Calcite Dolomite-ord Muscovite Kaolinite Quartz"
  constraint_value = "  1.0 1E-5 1.0 1.0 0.2  9.88249 3.652471265  1.2792268 1.2057878 226.99243"
  constraint_meaning = "kg_solvent_water activity moles_bulk_species free_molality free_molality free_moles_mineral_species free_moles_mineral_species free_moles_mineral_species free_moles_mineral_species free_moles_mineral_species"
  temperature = 70.0
  ramp_max_ionic_strength_initial = 0 # max_ionic_strength in such a simple problem does not need ramping
  precision = 8
[]

[UserObjects]
  [./definition]
    type = GeochemicalModelDefinition
    database_file = "../../../database/moose_geochemdb.json"
    basis_species = "H2O H+ Cl- Na+ Ca++ HCO3- Mg++ K+ Al+++ SiO2(aq)"
    equilibrium_minerals = "Calcite Dolomite-ord Muscovite Kaolinite Quartz"
  [../]
[]


[Postprocessors]
  [./pH]
    type = PointValue
    variable = "pH"
  [../]
  [./cm3_Calcite]
    type = PointValue
    variable = free_cm3_Calcite
  [../]
  [./cm3_Dolomite]
    type = PointValue
    variable = free_cm3_Dolomite-ord
  [../]
  [./cm3_Muscovite]
    type = PointValue
    variable = free_cm3_Muscovite
  [../]
  [./cm3_Kaolinite]
    type = PointValue
    variable = free_cm3_Kaolinite
  [../]
  [./cm3_Quartz]
    type = PointValue
    variable = free_cm3_Quartz
  [../]
[]

[Outputs]
  csv = true
[]
