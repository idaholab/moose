[UserObjects]
  [Basalt_rate]
    type = GeochemistryKineticRate
    kinetic_species_name = Basalt_SRP
    intrinsic_rate_constant = 1.44e-7 #1.44e-7 mol/s/cm^2
    area_quantity = 1 # cm^2/g
    multiply_by_mass = true
    promoting_species_names = 'H+'
    promoting_indices = '0.15'
    activation_energy = 32000
    direction = both
  []
  [definition]
    type = GeochemicalModelDefinition
    database_file = 'basalt_database.json'
    basis_species = 'H2O O2(aq) H+ HCO3- Ca++ Mg++ Na+ K+ Cl- SO4-- F- NO3- SiO2(aq) Al+++ Fe++'
    kinetic_minerals = 'Basalt_SRP'
    equilibrium_gases = 'CO2(g)'
    equilibrium_minerals = 'Calcite	Chalcedony	Goethite	Kaolinite	Nontronit-Mg	Siderite Ankerite'
    kinetic_rate_descriptions = 'Basalt_rate'
  []
  [nodal_void_volume_uo]
    type = NodalVoidVolume
    porosity = porosity
    execute_on = 'initial timestep_end'
  []
[]

[SpatialReactionSolver]
  model_definition = definition
  geochemistry_reactor_name = reactor
  charge_balance_species = 'HCO3-'
  swap_out_of_basis = 'H+ '
  swap_into_basis = 'CO2(g)'
  constraint_species = 'H2O                O2(aq)            CO2(g)          HCO3-            Ca++             Mg++              Na+              K+               Cl-              SO4--            F-                NO3-              SiO2(aq)          Al+++            Fe++         '
  constraint_value = '  0.9999983226825    0.0002061         9.478        0.409357         0.00056          0.00063           0.00035          7.9469e-5        0.00013          0.00016          1.00238E-5        6.46592E-6        0.00056           1e-12            1e-12        '
  constraint_meaning = 'kg_solvent_water   bulk_composition  fugacity     bulk_composition bulk_composition bulk_composition bulk_composition  bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition  bulk_composition  bulk_composition  bulk_composition '
  constraint_unit = '   kg                 moles    dimensionless    moles            moles            moles             moles            moles            moles            moles            moles             moles             moles             moles            moles        '
  initial_temperature = 20
  temperature = 20
  kinetic_species_name = 'Basalt_SRP'
  kinetic_species_initial_value = '34.16500994'
  kinetic_species_unit = 'moles'
  evaluate_kinetic_rates_always = true # otherwise will easily 'run out' of dissolving species
  source_species_names = 'H2O Al+++ Ca++ Cl- F- Fe++ H+ HCO3- K+ Mg++ Na+ NO3- O2(aq) SiO2(aq) SO4--'
  source_species_rates = 'rate_H2O_per_1l rate_Al_per_1l rate_Ca_per_1l rate_Cl_per_1l rate_F_per_1l rate_Fe_per_1l rate_H_per_1l rate_HCO3_per_1l rate_K_per_1l rate_Mg_per_1l rate_Na_per_1l rate_NO3_per_1l rate_O2_per_1l rate_SiO2_per_1l rate_SO4_per_1l'
  ramp_max_ionic_strength_initial = 0 # max_ionic_strength in such a simple problem does not need ramping
  controlled_activity_name = 'CO2(g)'
  controlled_activity_value = fug_co2
  stoichiometric_ionic_str_using_Cl_only = true # for comparison with GWB
  add_aux_pH = false
  execute_console_output_on = ''
  mol_cutoff = 1E-13
  add_aux_molal = false # save some memory and reduce variables in output exodus
  add_aux_mg_per_kg = false # save some memory and reduce variables in output exodus
  add_aux_free_mg = false # save some memory and reduce variables in output exodus
  add_aux_activity = false # save some memory and reduce variables in output exodus
  add_aux_bulk_moles = false # save some memory and reduce variables in output exodus
  adaptive_timestepping = true
[]

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 20
    ny = 10
    xmin = 0
    xmax = 0.0508 #m
    ymin = 0
    ymax = 0.0254 #m
  []
[]

[Executioner]
  type = Transient
  [TimeStepper]
    type = FunctionDT
    function = 'max(1E6, 0.3 * t)'
  []
  end_time = 4E12
[]


[AuxVariables]
  [temperature]
    initial_condition = 18.8
  []
  [porosity]
    initial_condition = 0.025
  []
  [nodal_void_volume]
  []
  [free_cm3_Basalt_SRP]
  []
  [transported_H2O]
  []
  [transported_Al]
  []
  [transported_Ca]
  []
  [transported_Cl]
  []
  [transported_F]
  []
  [transported_Fe]
  []
  [transported_H]
  []
  [transported_HCO3]
  []
  [transported_K]
  []
  [transported_Mg]
  []
  [transported_NO3]
  []
  [transported_Na]
  []
  [transported_O2]
  []
  [transported_SO4]
  []
  [transported_SiO2]
  []
  [transported_mass]
  []
  [pf_rate_H2O]
  []
  [pf_rate_Al]
  []
  [pf_rate_Ca]
  []
  [pf_rate_Cl]
  []
  [pf_rate_F]
  []
  [pf_rate_Fe]
  []
  [pf_rate_H]
  []
  [pf_rate_HCO3]
  []
  [pf_rate_K]
  []
  [pf_rate_Mg]
  []
  [pf_rate_NO3]
  []
  [pf_rate_Na]
  []
  [pf_rate_O2]
  []
  [pf_rate_SO4]
  []
  [pf_rate_SiO2]
  []
  [rate_H2O_per_1l]
  []
  [rate_Al_per_1l]
  []
  [rate_Ca_per_1l]
  []
  [rate_Cl_per_1l]
  []
  [rate_F_per_1l]
  []
  [rate_Fe_per_1l]
  []
  [rate_H_per_1l]
  []
  [rate_HCO3_per_1l]
  []
  [rate_K_per_1l]
  []
  [rate_Mg_per_1l]
  []
  [rate_NO3_per_1l]
  []
  [rate_Na_per_1l]
  []
  [rate_O2_per_1l]
  []
  [rate_SO4_per_1l]
  []
  [rate_SiO2_per_1l]
  []
  [massfrac_H2O]
  []
  [massfrac_Al]
  []
  [massfrac_Ca]
  []
  [massfrac_Cl]
  []
  [massfrac_F]
  []
  [massfrac_Fe]
  []
  [massfrac_H]
  []
  [massfrac_HCO3]
  []
  [massfrac_K]
  []
  [massfrac_Mg]
  []
  [massfrac_NO3]
  []
  [massfrac_Na]
  []
  [massfrac_O2]
  []
  [massfrac_SO4]
  []
  [massfrac_SiO2]
  []
  [fug_co2]
  []
  [pressure]
  []
[]

[AuxKernels]
  # [free_cm3_Basalt_SRP]
  #   type = GeochemistryQuantityAux
  #   variable = free_cm3_Basalt_SRP
  #   species = 'Basalt_SRP'
  #   quantity = free_cm3
  #   execute_on = 'timestep_begin timestep_end'
  # []
  # [porosity_auxk]
  #   type = ParsedAux
  #   coupled_variables = 'free_cm3_Basalt_SRP'
  #   expression = '1000.0 / (1000.0 + free_cm3_Basalt_SRP)'
  #   variable = porosity
  #   execute_on = 'timestep_end'
  # []
  [fug_co2]
    type = FunctionAux
    variable = fug_co2
    function = '0.9560*(pressure/1e5) + 0.13274'  #pascal to bar for fugacity calculation
    execute_on = timestep_begin # so the correct value is provided to the reactor
  []
  [nodal_void_volume_auxk]
    type = NodalVoidVolumeAux
    variable = nodal_void_volume
    nodal_void_volume_uo = nodal_void_volume_uo
    execute_on = 'initial timestep_end' # "initial" to ensure it is properly evaluated for the first timestep
  []
  [rate_H2O_per_1l_auxk]
    type = ParsedAux
    coupled_variables = 'pf_rate_H2O nodal_void_volume'
    variable = rate_H2O_per_1l
    expression = 'pf_rate_H2O / 18.01801802 / nodal_void_volume'
    execute_on = 'timestep_begin'
  []
  [rate_Al_per_1l_auxk]
    type = ParsedAux
    coupled_variables = 'pf_rate_Al nodal_void_volume'
    variable = rate_Al_per_1l
    expression = 'pf_rate_Al / 26.9815 / nodal_void_volume'
    execute_on = 'timestep_begin'
  []
  [rate_Ca_per_1l_auxk]
    type = ParsedAux
    coupled_variables = 'pf_rate_Ca nodal_void_volume'
    variable = rate_Ca_per_1l
    expression = 'pf_rate_Ca / 40.08 / nodal_void_volume'
    execute_on = 'timestep_begin'
  []
  [rate_Cl_per_1l_auxk]
    type = ParsedAux
    coupled_variables = 'pf_rate_Cl nodal_void_volume'
    variable = rate_Cl_per_1l
    expression = 'pf_rate_Cl / 35.453 / nodal_void_volume'
    execute_on = 'timestep_begin'
  []
  [rate_F_per_1l_auxk]
    type = ParsedAux
    coupled_variables = 'pf_rate_F nodal_void_volume'
    variable = rate_F_per_1l
    expression = 'pf_rate_F / 19.00 / nodal_void_volume'
    execute_on = 'timestep_begin'
  []
  [rate_Fe_per_1l_auxk]
    type = ParsedAux
    coupled_variables = 'pf_rate_Fe nodal_void_volume'
    variable = rate_Fe_per_1l
    expression = 'pf_rate_Fe /55.85/ nodal_void_volume'
    execute_on = 'timestep_begin'
  []
  [rate_H_per_1l_auxk]
    type = ParsedAux
    coupled_variables = 'pf_rate_H nodal_void_volume'
    variable = rate_H_per_1l
    expression = 'pf_rate_H / 1.0079 / nodal_void_volume'
    execute_on = 'timestep_begin'
  []
  [rate_HCO3_per_1l_auxk]
    type = ParsedAux
    coupled_variables = 'pf_rate_HCO3 nodal_void_volume'
    variable = rate_HCO3_per_1l
    expression = 'pf_rate_HCO3 / 61.0171 / nodal_void_volume'
    execute_on = 'timestep_begin'
  []
  [rate_K_per_1l_auxk]
    type = ParsedAux
    coupled_variables = 'pf_rate_K nodal_void_volume'
    variable = rate_K_per_1l
    expression = 'pf_rate_K / 39.0983 / nodal_void_volume'
    execute_on = 'timestep_begin'
  []
  [rate_Mg_per_1l_auxk]
    type = ParsedAux
    coupled_variables = 'pf_rate_Mg nodal_void_volume'
    variable = rate_Mg_per_1l
    expression = 'pf_rate_Mg / 24.305 / nodal_void_volume'
    execute_on = 'timestep_begin'
  []
  [rate_NO3_per_1l_auxk]
    type = ParsedAux
    coupled_variables = 'pf_rate_NO3 nodal_void_volume'
    variable = rate_NO3_per_1l
    expression = 'pf_rate_NO3 / 62.00 / nodal_void_volume'
    execute_on = 'timestep_begin'
  []
  [rate_Na_per_1l_auxk]
    type = ParsedAux
    coupled_variables = 'pf_rate_Na nodal_void_volume'
    variable = rate_Na_per_1l
    expression = 'pf_rate_Na / 22.9898 / nodal_void_volume'
    execute_on = 'timestep_begin'
  []
  [rate_O2_per_1l_auxk]
    type = ParsedAux
    coupled_variables = 'pf_rate_O2 nodal_void_volume'
    variable = rate_O2_per_1l
    expression = 'pf_rate_O2 / 32.00 / nodal_void_volume'
    execute_on = 'timestep_begin'
  []
  [rate_SO4_per_1l_auxk]
    type = ParsedAux
    coupled_variables = 'pf_rate_SO4 nodal_void_volume'
    variable = rate_SO4_per_1l
    expression = 'pf_rate_SO4 / 96.0576 / nodal_void_volume'
    execute_on = 'timestep_begin'
  []
  [rate_SiO2_per_1l_auxk]
    type = ParsedAux
    coupled_variables = 'pf_rate_SiO2 nodal_void_volume'
    variable = rate_SiO2_per_1l
    expression = 'pf_rate_SiO2 / 60.0843 / nodal_void_volume'
    execute_on = 'timestep_begin'
  []
  [transported_H2O_auxk]
    type = GeochemistryQuantityAux
    variable = transported_H2O
    species = H2O
    quantity = transported_moles_in_original_basis
  []
  [transported_Al]
    type = GeochemistryQuantityAux
    variable = transported_Al
    species = Al+++
    quantity = transported_moles_in_original_basis
  []
  [transported_Ca]
    type = GeochemistryQuantityAux
    variable = transported_Ca
    species = Ca++
    quantity = transported_moles_in_original_basis
  []
  [transported_Cl]
    type = GeochemistryQuantityAux
    variable = transported_Cl
    species = Cl-
    quantity = transported_moles_in_original_basis
  []
  [transported_F]
    type = GeochemistryQuantityAux
    variable = transported_F
    species = F-
    quantity = transported_moles_in_original_basis
  []
  [transported_Fe]
    type = GeochemistryQuantityAux
    variable = transported_Fe
    species = Fe++
    quantity = transported_moles_in_original_basis
  []
  [transported_H]
    type = GeochemistryQuantityAux
    variable = transported_H
    species = H+
    quantity = transported_moles_in_original_basis
  []
  [transported_HCO3]
    type = GeochemistryQuantityAux
    variable = transported_HCO3
    species = HCO3-
    quantity = transported_moles_in_original_basis
  []
  [transported_K]
    type = GeochemistryQuantityAux
    variable = transported_K
    species = K+
    quantity = transported_moles_in_original_basis
  []
  [transported_Mg]
    type = GeochemistryQuantityAux
    variable = transported_Mg
    species = Mg++
    quantity = transported_moles_in_original_basis
  []
  [transported_NO3]
    type = GeochemistryQuantityAux
    variable = transported_NO3
    species = NO3-
    quantity = transported_moles_in_original_basis
  []
  [transported_Na]
    type = GeochemistryQuantityAux
    variable = transported_Na
    species = Na+
    quantity = transported_moles_in_original_basis
  []
  [transported_O2]
    type = GeochemistryQuantityAux
    variable = transported_O2
    species = O2(aq)
    quantity = transported_moles_in_original_basis
  []
  [transported_SO4]
    type = GeochemistryQuantityAux
    variable = transported_SO4
    species = SO4--
    quantity = transported_moles_in_original_basis
  []
  [transported_SiO2]
    type = GeochemistryQuantityAux
    variable = transported_SiO2
    species = SiO2(aq)
    quantity = transported_moles_in_original_basis
  []
  [transported_mass_auxk]
    type = ParsedAux
    coupled_variables = ' transported_Al transported_Ca transported_Cl transported_F transported_Fe transported_H transported_HCO3 transported_K transported_Mg transported_NO3 transported_Na transported_O2 transported_SiO2 transported_SO4 transported_H2O'
    variable = transported_mass
    expression = 'transported_Al * 26.9815 + transported_Ca * 40.08 + transported_Cl * 35.453 + transported_F * 19 + transported_Fe * 55.85 + transported_H * 1.0079 + transported_HCO3 * 61.0171 + transported_K * 39.0983 + transported_Mg * 24.305 + transported_NO3 * 24.305 + transported_Na * 22.9898 + transported_O2 * 32 + transported_SiO2*96.0576 + transported_SO4*60.0843 + transported_H2O*18.01801802'
    execute_on = 'timestep_end'
  []
  [massfrac_H2O_auxk]
    type = ParsedAux
    coupled_variables = 'transported_H2O transported_mass'
    variable = massfrac_H2O
    expression = 'transported_H2O * 18.01801802 / transported_mass'
    execute_on = 'timestep_end'
  []
  [massfrac_Al_auxk]
    type = ParsedAux
    coupled_variables = 'transported_Al transported_mass'
    variable = massfrac_Al
    expression = 'transported_Al * 26.9815 / transported_mass'
    execute_on = 'timestep_end'
  []
  [massfrac_Ca_auxk]
    type = ParsedAux
    coupled_variables = 'transported_Ca transported_mass'
    variable = massfrac_Ca
    expression = 'transported_Ca * 40.08 / transported_mass'
    execute_on = 'timestep_end'
  []
  [massfrac_Cl_auxk]
    type = ParsedAux
    coupled_variables = 'transported_Cl transported_mass'
    variable = massfrac_Cl
    expression = 'transported_Cl * 35.453 / transported_mass'
    execute_on = 'timestep_end'
  []
  [massfrac_F_auxk]
    type = ParsedAux
    coupled_variables = 'transported_F transported_mass'
    variable = massfrac_F
    expression = 'transported_F * 19.00 / transported_mass'
    execute_on = 'timestep_end'
  []
  [massfrac_Fe_auxk]
    type = ParsedAux
    coupled_variables = 'transported_Fe transported_mass'
    variable = massfrac_Fe
    expression = 'transported_Fe * 55.85 / transported_mass'
    execute_on = 'timestep_end'
  []
  [massfrac_H_auxk]
    type = ParsedAux
    coupled_variables = 'transported_H transported_mass'
    variable = massfrac_H
    expression = 'transported_H * 1.0079 / transported_mass'
    execute_on = 'timestep_end'
  []
  [massfrac_HCO3_auxk]
    type = ParsedAux
    coupled_variables = 'transported_HCO3 transported_mass'
    variable = massfrac_HCO3
    expression = 'transported_HCO3 * 61.0171 / transported_mass'
    execute_on = 'timestep_end'
  []
  [massfrac_K_auxk]
    type = ParsedAux
    coupled_variables = 'transported_K transported_mass'
    variable = massfrac_K
    expression = 'transported_K * 39.0983 / transported_mass'
    execute_on = 'timestep_end'
  []
  [massfrac_Mg_auxk]
    type = ParsedAux
    coupled_variables = 'transported_Mg transported_mass'
    variable = massfrac_Mg
    expression = 'transported_Mg * 24.305 / transported_mass'
    execute_on = 'timestep_end'
  []
  [massfrac_NO3_auxk]
    type = ParsedAux
    coupled_variables = 'transported_NO3 transported_mass'
    variable = massfrac_NO3
    expression = 'transported_NO3 * 62 / transported_mass'
    execute_on = 'timestep_end'
  []
  [massfrac_Na_auxk]
    type = ParsedAux
    coupled_variables = 'transported_Na transported_mass'
    variable = massfrac_Na
    expression = 'transported_Na * 22.9898 / transported_mass'
    execute_on = 'timestep_end'
  []
  [massfrac_O2_auxk]
    type = ParsedAux
    coupled_variables = 'transported_O2 transported_mass'
    variable = massfrac_O2
    expression = 'transported_O2 * 32 / transported_mass'
    execute_on = 'timestep_end'
  []
  [massfrac_SO4_auxk]
    type = ParsedAux
    coupled_variables = 'transported_SO4 transported_mass'
    variable = massfrac_SO4
    expression = 'transported_SO4 * 96.0576 / transported_mass'
    execute_on = 'timestep_end'
  []
  [massfrac_SiO2_auxk]
    type = ParsedAux
    coupled_variables = 'transported_SiO2 transported_mass'
    variable = massfrac_SiO2
    expression = 'transported_SiO2 * 60.0843 / transported_mass'
    execute_on = 'timestep_end'
  []
[]

[GlobalParams]
  point = '0 0 0'
  reactor = reactor
[]

[Postprocessors]
  # [mole_H2O]
  #   type = PointValue
  #   variable = transported_H2O
  # []
  # [mole_Al]
  #   type = PointValue
  #   variable = transported_Al
  # []
  # [mole_Ca]
  #   type = PointValue
  #   variable = transported_Ca
  # []
  # [mole_Cl]
  #   type = PointValue
  #   variable = transported_Cl
  # []
  # [mole_F]
  #   type = PointValue
  #   variable = transported_F
  # []
  # [mole_Fe]
  #   type = PointValue
  #   variable = transported_Fe
  # []
  # [mole_H]
  #   type = PointValue
  #   variable = transported_H
  # []
  # [mole_HCO3]
  #   type = PointValue
  #   variable = transported_HCO3
  # []
  # [mole_K]
  #   type = PointValue
  #   variable = transported_K
  # []
  # [mole_Mg]
  #   type = PointValue
  #   variable = transported_Mg
  # []
  # [mole_NO3]
  #   type = PointValue
  #   variable = transported_NO3
  # []
  # [mole_Na]
  #   type = PointValue
  #   variable = transported_Na
  # []
  # [mole_O2]
  #   type = PointValue
  #   variable = transported_O2
  # []
  # [mole_SO4]
  #   type = PointValue
  #   variable = transported_SO4
  # []
  # [mole_SiO2]
  #   type = PointValue
  #   variable = transported_SiO2
  # []
  # [reaction_basalt]
  #   type = PointValue
  #   variable = rxn_rate
  # []
  [temperature]
    type = PointValue
    variable = 'solution_temperature'
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
  [massfrac_Al]
    type = PointValue
    variable = massfrac_Al
  []
  [massfrac_Ca]
    type = PointValue
    variable = massfrac_Ca
  []
  [massfrac_Cl]
    type = PointValue
    variable = massfrac_Cl
  []
  [massfrac_F]
    type = PointValue
    variable = massfrac_F
  []
  [massfrac_Fe]
    type = PointValue
    variable = massfrac_Fe
  []
  [massfrac_H]
    type = PointValue
    variable = massfrac_H
  []
  [massfrac_HCO3]
    type = PointValue
    variable = massfrac_HCO3
  []
  [massfrac_K]
    type = PointValue
    variable = massfrac_K
  []
  [massfrac_Mg]
    type = PointValue
    variable = massfrac_Mg
  []
  [massfrac_NO3]
    type = PointValue
    variable = massfrac_NO3
  []
  [massfrac_Na]
    type = PointValue
    variable = massfrac_Na
  []
  [massfrac_O2]
    type = PointValue
    variable = massfrac_O2
  []
  [massfrac_SO4]
    type = PointValue
    variable = massfrac_SO4
  []
  [massfrac_SiO2]
    type = PointValue
    variable = massfrac_SiO2
  []
[]

[Outputs]
  # [specially_added]
  #   type = GeochemistryConsoleOutput
  #   geochemistry_reactor = reactor
  #   precision = 8 # 8 digits of precision
  #   mol_cutoff = 1E-8 # species with molality or mole-number lower than this are not outputted
  #   solver_info = true
  #   nearest_node_number_UO = nearest_node
  #   execute_on = 'TIMESTEP_END' # just output at the end of the simulation
  # []
  [exo]
    type = Exodus
    execute_on = final
  []
  csv = true
[]
