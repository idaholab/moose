# An example of manually adding a GeochemistryConsoleOutput
[TimeDependentReactionSolver]
  model_definition = definition
  geochemistry_reactor_name = reactor_name
  charge_balance_species = "Cl-"
  constraint_species = "H2O H+ Cl- Fe+++"
  constraint_value = "  1.0 1.0E-2 1.3E-2 1.0E-3"
  constraint_meaning = "kg_solvent_water bulk_composition bulk_composition bulk_composition"
  constraint_unit = "kg moles moles moles"
  ramp_max_ionic_strength_initial = 0
  ramp_max_ionic_strength_subsequent = 1 # so can see the solver_info in the GeochemistryConsoleOutput
  kinetic_species_name = "Fe(OH)3(ppd)_nosorb"
  kinetic_species_initial_value = "1.0E-6"
  kinetic_species_unit = moles
  execute_console_output_on = 'final' # can compare with the specially_added one, below
[]

[UserObjects]
  [definition]
    type = GeochemicalModelDefinition
    database_file = "../../database/ferric_hydroxide_sorption.json"
    basis_species = "H2O H+ Cl- Fe+++"
    kinetic_minerals = "Fe(OH)3(ppd)_nosorb"
  []
  [nearest_node]
    type = NearestNodeNumberUO
    point = '0 0 0' # in this case there is no spatial dependence, so the point is rather irrelevant
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  [specially_added]
    type = GeochemistryConsoleOutput
    geochemistry_reactor = reactor_name
    precision = 8 # 8 digits of precision
    mol_cutoff = 1E-8 # species with molality or mole-number lower than this are not outputted
    solver_info = true
    nearest_node_number_UO = nearest_node
    execute_on = 'final' # just output at the end of the simulation
  []
[]
