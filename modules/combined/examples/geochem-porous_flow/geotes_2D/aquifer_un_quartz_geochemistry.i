[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 56
    ny = 32
    xmin = -70
    xmax = 70
    ymin = -40
    ymax = 40
  []
[]

[GlobalParams]
  point = '0 0 0'
  reactor = reactor
[]

[SpatialReactionSolver]
  model_definition = definition
  geochemistry_reactor_name = reactor
  charge_balance_species = "Cl-"
  constraint_species = "H2O              Na+                Cl-                SiO2(aq)"
# ASSUME that 1 litre of solution contains:
  constraint_value = "  1.0              0.1                0.1                0.00172249633"
  constraint_meaning = "kg_solvent_water bulk_composition bulk_composition free_concentration"
  constraint_unit = "   kg               moles              moles              molal"
  initial_temperature = 50.0
  kinetic_species_name = QuartzUnlike
# Per 1 litre (1000cm^3) of aqueous solution (1kg of solvent water), there is 9000cm^3 of QuartzUnlike, which means the initial porosity is 0.1.
  kinetic_species_initial_value = 9000
  kinetic_species_unit = cm3
  temperature = temperature
  source_species_names = 'H2O    Na+   Cl-   SiO2(aq)'
  source_species_rates = 'rate_H2O_per_1l rate_Na_per_1l rate_Cl_per_1l rate_SiO2_per_1l'
  ramp_max_ionic_strength_initial = 0 # max_ionic_strength in such a simple problem does not need ramping
  add_aux_pH = false # there is no H+ in this system
  evaluate_kinetic_rates_always = true # implicit time-marching used for stability
  execute_console_output_on = '' # only CSV and exodus output used in this example
[]

[UserObjects]
  [rate_quartz]
    type = GeochemistryKineticRate
    kinetic_species_name = QuartzUnlike
    intrinsic_rate_constant = 1.0E-2
    multiply_by_mass = true
    area_quantity = 1
    activation_energy = 72800.0
  []
  [definition]
    type = GeochemicalModelDefinition
    database_file = "small_database.json"
    basis_species = "H2O SiO2(aq) Na+ Cl-"
    kinetic_minerals = "QuartzUnlike"
    kinetic_rate_descriptions = "rate_quartz"
  []
[]


[Executioner]
  type = Transient
  dt = 1E5
  end_time = 7.76E6 # 90 days
[]

[AuxVariables]
  [temperature]
    initial_condition = 50.0
  []
  [nodal_volume]
  []
  [porosity]
  []
  [nodal_void_volume]
  []
  [pf_rate_H2O] # change in H2O mass (kg/s) at each node provided by the porous-flow simulation
  []
  [pf_rate_Na] # change in H2O mass (kg/s) at each node provided by the porous-flow simulation
  []
  [pf_rate_Cl] # change in H2O mass (kg/s) at each node provided by the porous-flow simulation
  []
  [pf_rate_SiO2] # change in H2O mass (kg/s) at each node provided by the porous-flow simulation
  []
  [rate_H2O_per_1l] # rate per 1 litre of aqueous solution that we consider at each node
  []
  [rate_Na_per_1l]
  []
  [rate_Cl_per_1l]
  []
  [rate_SiO2_per_1l]
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
  [massfrac_Na]
  []
  [massfrac_Cl]
  []
  [massfrac_SiO2]
  []
  [massfrac_H2O]
  []
[]
[AuxKernels]
  [nodal_volume] # TODO: change this hard-coded version once PR is merged
    type = FunctionAux
    variable = nodal_volume
    function = 'if(abs(x) = 70 & abs(y) = 40, 2.5, if(abs(x) = 70 | abs(y) = 40, 5, 10))'
    execute_on = 'initial'
  []
  [porosity]
    type = ParsedAux
    coupled_variables = free_cm3_QuartzUnlike
    expression = '1000.0 / (1000.0 + free_cm3_QuartzUnlike)'
    variable = porosity
    execute_on = 'timestep_begin timestep_end'
  []
  [nodal_void_volume]
    type = ParsedAux
    coupled_variables = 'porosity nodal_volume'
    variable = nodal_void_volume
    expression = 'porosity * nodal_volume'
    execute_on = 'timestep_begin'
  []
  [rate_H2O_per_1l]
    type = ParsedAux
    coupled_variables = 'pf_rate_H2O nodal_void_volume'
    variable = rate_H2O_per_1l
# pf_rate = change in kg at every node
# pf_rate * 1000 / molar_mass_in_g_per_mole = change in moles at every node
# pf_rate * 1000 / molar_mass / (nodal_void_volume_in_m^3 * 1000) = change in moles per litre of aqueous solution
    expression = 'pf_rate_H2O / 18.0152 / nodal_void_volume'
    execute_on = 'timestep_begin'
  []
  [rate_Na_per_1l]
    type = ParsedAux
    coupled_variables = 'pf_rate_Na nodal_void_volume'
    variable = rate_Na_per_1l
    expression = 'pf_rate_Na / 22.9898 / nodal_void_volume'
    execute_on = 'timestep_begin'
  []
  [rate_Cl_per_1l]
    type = ParsedAux
    coupled_variables = 'pf_rate_Cl nodal_void_volume'
    variable = rate_Cl_per_1l
    expression = 'pf_rate_Cl / 35.453 / nodal_void_volume'
    execute_on = 'timestep_begin'
  []
  [rate_SiO2_per_1l]
    type = ParsedAux
    coupled_variables = 'pf_rate_SiO2 nodal_void_volume'
    variable = rate_SiO2_per_1l
    expression = 'pf_rate_SiO2 / 60.0843 / nodal_void_volume'
    execute_on = 'timestep_begin'
  []
  [transported_H2O]
    type = GeochemistryQuantityAux
    variable = transported_H2O
    species = H2O
    quantity = transported_moles_in_original_basis
    execute_on = 'timestep_end'
  []
  [transported_Na]
    type = GeochemistryQuantityAux
    variable = transported_Na
    species = Na+
    quantity = transported_moles_in_original_basis
    execute_on = 'timestep_end'
  []
  [transported_Cl]
    type = GeochemistryQuantityAux
    variable = transported_Cl
    species = Cl-
    quantity = transported_moles_in_original_basis
    execute_on = 'timestep_end'
  []
  [transported_SiO2]
    type = GeochemistryQuantityAux
    variable = transported_SiO2
    species = 'SiO2(aq)'
    quantity = transported_moles_in_original_basis
    execute_on = 'timestep_end'
  []
  [transported_mass]
    type = ParsedAux
    coupled_variables = 'transported_H2O transported_Na transported_Cl transported_SiO2'
    variable = transported_mass
    expression = 'transported_H2O * 18.0152 + transported_Na * 22.9898 + transported_Cl * 35.453 + transported_SiO2 * 60.0843'
    execute_on = 'timestep_end'
  []
  [massfrac_H2O]
    type = ParsedAux
    coupled_variables = 'transported_H2O transported_mass'
    variable = massfrac_H2O
    expression = 'transported_H2O * 18.0152 / transported_mass'
    execute_on = 'timestep_end'
  []
  [massfrac_Na]
    type = ParsedAux
    coupled_variables = 'transported_Na transported_mass'
    variable = massfrac_Na
    expression = 'transported_Na * 22.9898 / transported_mass'
    execute_on = 'timestep_end'
  []
  [massfrac_Cl]
    type = ParsedAux
    coupled_variables = 'transported_Cl transported_mass'
    variable = massfrac_Cl
    expression = 'transported_Cl * 35.453 / transported_mass'
    execute_on = 'timestep_end'
  []
  [massfrac_SiO2]
    type = ParsedAux
    coupled_variables = 'transported_SiO2 transported_mass'
    variable = massfrac_SiO2
    expression = 'transported_SiO2 * 60.0843 / transported_mass'
    execute_on = 'timestep_end'
  []
[]
[Postprocessors]
  [cm3_quartz]
    type = PointValue
    variable = free_cm3_QuartzUnlike
  []
  [porosity]
    type = PointValue
    variable = porosity
  []
  [solution_temperature]
    type = PointValue
    variable = solution_temperature
  []
  [massfrac_H2O]
    type = PointValue
    variable = massfrac_H2O
  []
  [massfrac_Na]
    type = PointValue
    variable = massfrac_Na
  []
  [massfrac_Cl]
    type = PointValue
    variable = massfrac_Cl
  []
  [massfrac_SiO2]
    type = PointValue
    variable = massfrac_SiO2
  []
[]
[Outputs]
  exodus = true
  csv = true
[]
