# Sorption onto FerricHydroxide along with changing pH
# There is 1 free gram of Fe(OH)3(ppd), which amounts to 9.357E-3 free moles.
# Per mole of Fe(OH)3(ppd) there are 0.005 moles of >(s)FeOH, giving a total of 4.679E-5 moles (bulk composition)
# Per mole of Fe(OH)3(ppd) there are 0.2 moles of >(w)FeOH, giving a total of 1.871E-3 moles (bulk composition)
[TimeDependentReactionSolver]
  model_definition = definition
  geochemistry_reactor_name = reactor
  swap_out_of_basis = "Fe+++"
  swap_into_basis = "Fe(OH)3(ppd)"
  charge_balance_species = "Cl-"
  constraint_species = "H2O              H+        Na+ Cl- Fe(OH)3(ppd) >(s)FeOH  >(w)FeOH"
  constraint_value = "  1.0              1E-4      0.1 0.1 9.3573E-3    4.6786E-5 1.87145E-3"
  constraint_meaning = "kg_solvent_water activity  moles_bulk_species moles_bulk_species free_moles_mineral_species moles_bulk_species moles_bulk_species"
  abs_tol = 1E-14
  controlled_activity_name = "H+"
  controlled_activity_value = set_aH
  execute_console_output_on = 'initial final'
[]

[AuxVariables]
  [./set_aH]
  [../]
[]
[AuxKernels]
  [./set_aH]
    type = FunctionAux
    variable = set_aH
    function = '10^(-4-t)'
    execute_on = timestep_begin # so the correct value is provided to the reactor
  [../]
[]

[Postprocessors]
  [./pH]
    type = PointValue
    point = '0 0 0'
    variable = 'pH'
  [../]
  [./molal_>wFeOH2+]
    type = PointValue
    point = '0 0 0'
    variable = 'molal_>(w)FeOH2+'
  [../]
  [./molal_>wFeOH]
    type = PointValue
    point = '0 0 0'
    variable = 'molal_>(w)FeOH'
  [../]
  [./molal_>wFeO-]
    type = PointValue
    point = '0 0 0'
    variable = 'molal_>(w)FeO-'
  [../]
  [./molal_>sFeOH2+]
    type = PointValue
    point = '0 0 0'
    variable = 'molal_>(s)FeOH2+'
  [../]
  [./molal_>sFeOH]
    type = PointValue
    point = '0 0 0'
    variable = 'molal_>(s)FeOH'
  [../]
  [./molal_>sFeO-]
    type = PointValue
    point = '0 0 0'
    variable = 'molal_>(s)FeO-'
  [../]
  [./potential]
    type = PointValue
    point = '0 0 0'
    variable = 'surface_potential_Fe(OH)3(ppd)'
  [../]
[]

[Executioner]
  type = Transient
  start_time = -1
  dt = 1
  end_time = 8
[]

[UserObjects]
  [./definition]
    type = GeochemicalModelDefinition
    database_file = "../../database/ferric_hydroxide_sorption.json"
    basis_species = "H2O H+ Na+ Cl- Fe+++ >(s)FeOH >(w)FeOH"
    equilibrium_minerals = "Fe(OH)3(ppd)"
  [../]
[]

