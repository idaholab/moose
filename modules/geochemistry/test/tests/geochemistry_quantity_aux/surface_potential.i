#Extract surface potential
[TimeIndependentReactionSolver]
  model_definition = definition
  swap_out_of_basis = 'H+'
  swap_into_basis = 'Fe(OH)3(ppd)'
  charge_balance_species = "Cl-"
  constraint_species = "H2O Cl- Fe+++ >(s)FeOH >(w)FeOH Fe(OH)3(ppd)"
  constraint_value = "  1.0 1.0E-6 1.0E-6 1.0E-6 1.0E-6 1.0E-6"
  constraint_meaning = "kg_solvent_water bulk_composition bulk_composition free_concentration free_concentration free_mineral"
  constraint_unit = "kg moles moles molal molal moles"
  ramp_max_ionic_strength_initial = 0
  abs_tol = 1E-20
[]

[UserObjects]
  [definition]
    type = GeochemicalModelDefinition
    database_file = "../../database/ferric_hydroxide_sorption.json"
    basis_species = "H2O H+ Cl- Fe+++ >(s)FeOH >(w)FeOH"
    equilibrium_minerals = "Fe(OH)3(ppd)"
  []
[]

[AuxVariables]
  [the_aux]
  []
[]

[AuxKernels]
  [the_aux]
    type = GeochemistryQuantityAux
    species = "Fe(OH)3(ppd)"
    reactor = geochemistry_reactor
    variable = the_aux
    quantity = surface_potential
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
    variable = "surface_potential_Fe(OH)3(ppd)"
  []
[]

[Outputs]
  csv = true
[]



