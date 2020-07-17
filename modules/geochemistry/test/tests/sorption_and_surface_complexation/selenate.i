# Langmuir sorption of Selenate
[TimeIndependentReactionSolver]
  model_definition = definition
  charge_balance_species = "Na+"
  constraint_species = "H2O              H+           Na+                SorbingSite        SeO4--"
  constraint_value = "  1.0              3.1622777E-8 10E-6              310E-9             5E-6"
  constraint_meaning = "kg_solvent_water activity     moles_bulk_species moles_bulk_species free_molality"
  ramp_max_ionic_strength_initial = 0 # not needed in this simple problem
  execute_console_output_on = '' # only CSV output for this problem
[]

[UserObjects]
  [./definition]
    type = GeochemicalModelDefinition
    database_file = "../../database/selenate_sorption.json"
    basis_species = "H2O H+ Na+ SeO4-- SorbingSite"
  [../]
[]

[AuxVariables]
  [./mol_sorbed_selenate_per_g_dry_soil]
  [../]
[]
[AuxKernels]
  [./mol_sorbed_selenate_per_g_dry_soil]
    type = ParsedAux
    args = molal_SorbedSelenate
    function = 'molal_SorbedSelenate / 500.0'
    variable = mol_sorbed_selenate_per_g_dry_soil
  [../]
[]
[Postprocessors]
  [./mol_sorbed_selenate_per_g_dry_soil]
    type = PointValue
    point = '0 0 0'
    variable = mol_sorbed_selenate_per_g_dry_soil
  [../]
[]

[Outputs]
  csv = true
[]
