#Extract kinetic moles
[TimeDependentReactionSolver]
  model_definition = definition
  charge_balance_species = "Cl-"
  constraint_species = "H2O H+ Cl- Fe+++ >(s)FeOH >(w)FeOH"
  constraint_value = "  1.0 4.0 1.0 0.1 1.0E-6 1.0E-6"
  constraint_meaning = "kg_solvent_water moles_bulk_species moles_bulk_species free_molality free_molality free_molality"
  kinetic_species_name = "Fe(OH)3(ppd)"
  kinetic_species_initial_moles = "1.0"
  max_ionic_strength = 0.0
  ramp_max_ionic_strength_initial = 0
[]

[UserObjects]
  [./definition]
    type = GeochemicalModelDefinition
    database_file = "../../database/ferric_hydroxide_sorption.json"
    basis_species = "H2O H+ Cl- Fe+++ >(s)FeOH >(w)FeOH"
    kinetic_minerals = "Fe(OH)3(ppd)"
  [../]
[]

[Executioner]
  type = Transient
  dt = 1
  end_time = 1
[]

[AuxVariables]
  [./the_aux]
  [../]
[]

[AuxKernels]
  [./the_aux]
    type = GeochemistryQuantityAux
    species = "Fe(OH)3(ppd)"
    reactor = geochemistry_reactor
    variable = the_aux
    quantity = kinetic_moles
  [../]
[]

[Postprocessors]
  [./value]
    type = PointValue
    point = '0 0 0'
    variable = the_aux
  [../]
  [./value_from_action]
    type = PointValue
    point = '0 0 0'
    variable = "moles_Fe(OH)3(ppd)"
  [../]
[]

[Outputs]
  csv = true
[]


