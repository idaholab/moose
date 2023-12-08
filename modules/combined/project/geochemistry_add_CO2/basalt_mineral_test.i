[GlobalParams]
  point = '0 0 0'
  reactor = reactor
[]

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
  [nearest_node]
    type = NearestNodeNumberUO
    point = '0 0 0' # in this case there is no spatial dependence, so the point is rather irrelevant
  []
[]

[TimeDependentReactionSolver]
  model_definition = definition
  geochemistry_reactor_name = reactor
  charge_balance_species = 'HCO3-'
  swap_out_of_basis = 'H+ '
  swap_into_basis = 'CO2(g)'
  constraint_species = 'H2O                O2(aq)            CO2(g)          HCO3-            Ca++             Mg++              Na+              K+               Cl-              SO4--            F-                NO3-              SiO2(aq)          Al+++            Fe++         '
  constraint_value = '  0.9999983226825    0.0002061          9.478         0.409357         0.00056          0.00063           0.00035          7.9469e-5        0.00013          0.00016          1.00238E-5        6.46592E-6        0.00056           1e-12            1e-12        '
  constraint_meaning = 'kg_solvent_water   bulk_composition  fugacity     bulk_composition bulk_composition bulk_composition bulk_composition  bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition  bulk_composition  bulk_composition  bulk_composition '
  constraint_unit = '   kg                 moles    dimensionless    moles            moles            moles             moles            moles            moles            moles            moles             moles             moles             moles            moles        '
  initial_temperature = 20.0
  temperature = 20.0
  kinetic_species_name = 'Basalt_SRP'
  kinetic_species_initial_value = '34.16500994'
  kinetic_species_unit = 'moles'
  # source_species_names = 'HCO3-'
  # source_species_rates = -20.0
  evaluate_kinetic_rates_always = true # otherwise will easily 'run out' of dissolving species
  ramp_max_ionic_strength_initial = 0 # max_ionic_strength in such a simple problem does not need ramping
  stoichiometric_ionic_str_using_Cl_only = true # for comparison with GWB
  add_aux_pH = true
  execute_console_output_on = ''
  mol_cutoff = 1E-13
[]

[Executioner]
  type = Transient
  dt = 10
  end_time = 10 #31557600 #1.9769E+04 #31557600
[]


[AuxVariables]
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
  [rxn_rate]
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
  [porosity]
    initial_condition = 0.127
  []
[]

[AuxKernels]
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
  [reaction_rate]
    type = GeochemistryQuantityAux
    variable = rxn_rate
    species = Basalt_SRP
    quantity = kinetic_moles
  []
  [transported_mass_auxk]
    type = ParsedAux
    coupled_variables = ' transported_Al transported_Ca transported_Cl transported_F transported_Fe transported_H transported_HCO3 transported_K transported_Mg transported_NO3 transported_Na transported_O2 transported_SiO2 transported_SO4 transported_H2O'
    variable = transported_mass
    expression = 'transported_Al * 26.9815 + transported_Ca * 40.08 + transported_Cl * 35.453 + transported_F * 19 + transported_Fe * 55.85 + transported_H * 1.0079 + transported_HCO3 * 61.0171 + transported_K * 39.0983 + transported_Mg * 24.305 + transported_NO3 * 62 + transported_Na * 22.9898 + transported_O2 * 32 + transported_SiO2*60.0843 + transported_SO4*96.0576 + transported_H2O*18.01801802'
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

[Postprocessors]
  [mole_H2O]
    type = PointValue
    variable = transported_H2O
  []
  [mole_Al]
    type = PointValue
    variable = transported_Al
  []
  [mole_Ca]
    type = PointValue
    variable = transported_Ca
  []
  [mole_Cl]
    type = PointValue
    variable = transported_Cl
  []
  [mole_F]
    type = PointValue
    variable = transported_F
  []
  [mole_Fe]
    type = PointValue
    variable = transported_Fe
  []
  [mole_H]
    type = PointValue
    variable = transported_H
  []
  [mole_HCO3]
    type = PointValue
    variable = transported_HCO3
  []
  [mole_K]
    type = PointValue
    variable = transported_K
  []
  [mole_Mg]
    type = PointValue
    variable = transported_Mg
  []
  [mole_NO3]
    type = PointValue
    variable = transported_NO3
  []
  [mole_Na]
    type = PointValue
    variable = transported_Na
  []
  [mole_O2]
    type = PointValue
    variable = transported_O2
  []
  [mole_SO4]
    type = PointValue
    variable = transported_SO4
  []
  [mole_SiO2]
    type = PointValue
    variable = transported_SiO2
  []
  [reaction_basalt]
    type = PointValue
    variable = rxn_rate
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
  [specially_added]
    type = GeochemistryConsoleOutput
    geochemistry_reactor = reactor
    precision = 8 # 8 digits of precision
    mol_cutoff = 1E-8 # species with molality or mole-number lower than this are not outputted
    solver_info = true
    nearest_node_number_UO = nearest_node
    execute_on = 'TIMESTEP_END' # just output at the end of the simulation
  []
  csv = true
[]
