#Extract free mg
[TimeIndependentReactionSolver]
  model_definition = definition
  swap_out_of_basis = 'H+'
  swap_into_basis = 'Fe(OH)3(ppd)_nosorb'
  charge_balance_species = "Cl-"
  constraint_species = "H2O Cl- Fe+++ Fe(OH)3(ppd)_nosorb"
  constraint_value = "  1.0 1.0E-6 1.0E-6 1.0"
  constraint_meaning = "kg_solvent_water bulk_composition free_concentration free_mineral"
  constraint_unit = "kg moles molal moles"
  ramp_max_ionic_strength_initial = 0
[]

[UserObjects]
  [definition]
    type = GeochemicalModelDefinition
    database_file = "../../database/ferric_hydroxide_sorption.json"
    basis_species = "H2O H+ Cl- Fe+++"
    equilibrium_minerals = "Fe(OH)3(ppd)_nosorb"
  []
[]

[AuxVariables]
  [the_aux]
  []
[]

[AuxKernels]
  [the_aux]
    type = GeochemistryQuantityAux
    species = "Fe(OH)3(ppd)_nosorb"
    reactor = geochemistry_reactor
    variable = the_aux
    quantity = free_mg
  []
[]

[Postprocessors]
  [value]
    type = PointValue
    point = '0 0 0'
    variable = the_aux
  []
  [value_from_action]
    type = PointValue
    point = '0 0 0'
    variable = "free_mg_Fe(OH)3(ppd)_nosorb"
  []
[]

[Outputs]
  csv = true
[]



