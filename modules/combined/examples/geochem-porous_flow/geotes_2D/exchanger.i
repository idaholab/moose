# Model of the heat-exchanger
# The input fluid to the heat exchanger is determined by AuxVariables called production_temperature, production_rate_Na, production_rate_Cl, production_rate_SiO2 and production_rate_H2O.  These come from Postprocessors in the porous-flow simulation that measure the fluid composition at the production well.
# Given the input fluid, the exchanger cools/heats the fluid, removing any precipitates, and injects fluid back to the porous-flow simulation at temperature output_temperature and composition given by massfrac_Na, etc.
# In the absence of data concerning Quartz precipitation rates in heat exchangers, do not treat Quartz as kinetic
[GlobalParams]
  point = '0 0 0'
  reactor = reactor
[]

[TimeDependentReactionSolver]
  model_definition = definition
  include_moose_solve = false
  geochemistry_reactor_name = reactor
  charge_balance_species = "Cl-"
  swap_out_of_basis = "SiO2(aq)"
  swap_into_basis = "QuartzLike"
  constraint_species = "H2O              Na+              Cl-              QuartzLike"
  constraint_value = "  1.0E-2           0.1E-2           0.1E-2           1E-10"
  constraint_meaning = "kg_solvent_water bulk_composition bulk_composition free_mineral"
  constraint_unit = "   kg               moles            moles            moles"
  initial_temperature = 50.0
  mode = 4
  temperature = 200
  cold_temperature = 40.0
  source_species_names = 'H2O    Na+   Cl-   SiO2(aq)'
  source_species_rates = 'production_rate_H2O production_rate_Na production_rate_Cl production_rate_SiO2'
  ramp_max_ionic_strength_initial = 0 # max_ionic_strength in such a simple problem does not need ramping
  add_aux_pH = false # there is no H+ in this system
  evaluate_kinetic_rates_always = true # implicit time-marching used for stability
  execute_console_output_on = '' # only CSV output used in this example
[]

[UserObjects]
  [definition]
    type = GeochemicalModelDefinition
    database_file = "small_database.json"
    basis_species = "H2O SiO2(aq) Na+ Cl-"
    equilibrium_minerals = "QuartzLike"
  []
[]

[Executioner]
  type = Transient
  dt = 1E5
  end_time = 2E6 #7.76E6 # 90 days
[]

[AuxVariables]
  [production_temperature]
    initial_condition = 50 # the production_T Transfer lags one timestep behind for some reason, so give this a reasonable initial condition
  []
  [transported_H2O]
  []
  [transported_Na]
  []
  [transported_Cl]
  []
  [transported_SiO2]
  []
  [transported_mass]
  []
  [massfrac_H2O]
  []
  [massfrac_Na]
  []
  [massfrac_Cl]
  []
  [massfrac_SiO2]
  []
  [dumped_quartz]
  []
  [production_rate_H2O]
    initial_condition = 5.518533e+01 # the production_H2O Transfer lags one timestep behind for some reason (when the porous_flow simulation has finished, it correctly computes mole_rate_H2O_produced, but the Transfer gets the mole_rate_H2O_produced from the previous timestep), so give this a reasonable initial condition, otherwise this will be zero at the start of the simulation!
  []
  [production_rate_Na]
    initial_condition = 9.943302e-02
  []
  [production_rate_Cl]
    initial_condition = 9.943302e-02
  []
  [production_rate_SiO2]
    initial_condition = 2.340931e-04
  []
[]
[AuxKernels]
  [transported_H2O_auxk]
    type = GeochemistryQuantityAux
    variable = transported_H2O
    species = H2O
    quantity = transported_moles_in_original_basis
  []
  [transported_Na]
    type = GeochemistryQuantityAux
    variable = transported_Na
    species = Na+
    quantity = transported_moles_in_original_basis
  []
  [transported_Cl]
    type = GeochemistryQuantityAux
    variable = transported_Cl
    species = Cl-
    quantity = transported_moles_in_original_basis
  []
  [transported_SiO2]
    type = GeochemistryQuantityAux
    variable = transported_SiO2
    species = 'SiO2(aq)'
    quantity = transported_moles_in_original_basis
  []
  [transported_mass_auxk]
    type = ParsedAux
    coupled_variables = 'transported_H2O transported_Na transported_Cl transported_SiO2'
    variable = transported_mass
    expression = 'transported_H2O * 18.0152 + transported_Na * 22.9898 + transported_Cl * 35.453 + transported_SiO2 * 60.0843'
  []
  [massfrac_H2O]
    type = ParsedAux
    coupled_variables = 'transported_mass transported_H2O'
    variable = massfrac_H2O
    expression = '18.0152 * transported_H2O / transported_mass'
  []
  [massfrac_Na]
    type = ParsedAux
    coupled_variables = 'transported_mass transported_Na'
    variable = massfrac_Na
    expression = '22.9898 * transported_Na / transported_mass'
  []
  [massfrac_Cl]
    type = ParsedAux
    coupled_variables = 'transported_mass transported_Cl'
    variable = massfrac_Cl
    expression = '35.453 * transported_Cl / transported_mass'
  []
  [massfrac_SiO2]
    type = ParsedAux
    coupled_variables = 'transported_mass transported_SiO2'
    variable = massfrac_SiO2
    expression = '60.0843 * transported_SiO2 / transported_mass'
  []
  [dumped_quartz]
    type = GeochemistryQuantityAux
    variable = dumped_quartz
    species = QuartzLike
    quantity = moles_dumped
  []
[]
[Postprocessors]
  [cumulative_moles_precipitated_quartz]
    type = PointValue
    variable = dumped_quartz
  []
  [production_temperature]
    type = PointValue
    variable = production_temperature
  []
  [mass_heated_this_timestep]
    type = PointValue
    variable = transported_mass
  []
[]
[Outputs]
  csv = true
[]

[MultiApps]
  [porous_flow_sim]
    type = TransientMultiApp
    input_files = porous_flow.i
    execute_on = 'timestep_end'
  []
[]

[Transfers]
  [injection_T]
    type = MultiAppNearestNodeTransfer
    direction = TO_MULTIAPP
    multi_app = porous_flow_sim
    fixed_meshes = true
    source_variable = 'solution_temperature'
    variable = 'injection_temperature'
  []
  [injection_Na]
    type = MultiAppNearestNodeTransfer
    direction = TO_MULTIAPP
    multi_app = porous_flow_sim
    fixed_meshes = true
    source_variable = 'massfrac_Na'
    variable = 'injection_rate_massfrac_Na'
  []
  [injection_Cl]
    type = MultiAppNearestNodeTransfer
    direction = TO_MULTIAPP
    multi_app = porous_flow_sim
    fixed_meshes = true
    source_variable = 'massfrac_Cl'
    variable = 'injection_rate_massfrac_Cl'
  []
  [injection_SiO2]
    type = MultiAppNearestNodeTransfer
    direction = TO_MULTIAPP
    multi_app = porous_flow_sim
    fixed_meshes = true
    source_variable = 'massfrac_SiO2'
    variable = 'injection_rate_massfrac_SiO2'
  []
  [injection_H2O]
    type = MultiAppNearestNodeTransfer
    direction = TO_MULTIAPP
    multi_app = porous_flow_sim
    fixed_meshes = true
    source_variable = 'massfrac_H2O'
    variable = 'injection_rate_massfrac_H2O'
  []

  [production_T]
    type = MultiAppPostprocessorInterpolationTransfer
    direction = FROM_MULTIAPP
    multi_app = porous_flow_sim
    postprocessor = production_temperature
    variable = production_temperature
  []
  [production_Na]
    type = MultiAppPostprocessorInterpolationTransfer
    direction = FROM_MULTIAPP
    multi_app = porous_flow_sim
    postprocessor = mole_rate_Na_produced
    variable = production_rate_Na
  []
  [production_Cl]
    type = MultiAppPostprocessorInterpolationTransfer
    direction = FROM_MULTIAPP
    multi_app = porous_flow_sim
    postprocessor = mole_rate_Cl_produced
    variable = production_rate_Cl
  []
  [production_SiO2]
    type = MultiAppPostprocessorInterpolationTransfer
    direction = FROM_MULTIAPP
    multi_app = porous_flow_sim
    postprocessor = mole_rate_SiO2_produced
    variable = production_rate_SiO2
  []
  [production_H2O]
    type = MultiAppPostprocessorInterpolationTransfer
    direction = FROM_MULTIAPP
    multi_app = porous_flow_sim
    postprocessor = mole_rate_H2O_produced
    variable = production_rate_H2O
  []
[]
