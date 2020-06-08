# An example of manually adding a GeochemistryConsoleOutput
[TimeIndependentReactionSolver]
  model_definition = definition
  geochemistry_reactor_name = reactor_name
  charge_balance_species = "Cl-"
  constraint_species = "H2O H+ Cl-"
  constraint_value = "  1.0 1.0E-2 1.0E-2"
  constraint_meaning = "kg_solvent_water moles_bulk_species moles_bulk_species"
  ramp_max_ionic_strength = 1 # so can see the solver_info in the GeochemistryConsoleOutput
[]

[UserObjects]
  [./definition]
    type = GeochemicalModelDefinition
    database_file = "../../database/ferric_hydroxide_sorption.json"
    basis_species = "H2O H+ Cl-"
  [../]
[]

[Outputs]
  [./specially_added]
    type = GeochemistryConsoleOutput
    geochemistry_reactor = reactor_name
    precision = 8 # 8 digits of precision
    mol_cutoff = 1E-8 # species with molality or mole-number lower than this are not outputted
    solver_info = true
    point = '1 0 0' # point at which the info is extracted: in this case the simulation is spatially-independent so the 'point' specification actually makes no difference
    execute_on = 'final' # just output at the end of the simulation
  [../]
[]
