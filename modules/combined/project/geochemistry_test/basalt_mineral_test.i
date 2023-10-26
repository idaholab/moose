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
  constraint_value = '  0.9999983226825    0.0002061         9.478        0.409357         0.00056          0.00063           0.00035          7.9469e-5        0.00013          0.00016          1.00238E-5        6.46592E-6        0.00056           1e-12            1e-12        '
  constraint_meaning = 'kg_solvent_water   bulk_composition  fugacity     bulk_composition bulk_composition bulk_composition bulk_composition  bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition  bulk_composition  bulk_composition  bulk_composition '
  constraint_unit = '   kg                 moles    dimensionless    moles            moles            moles             moles            moles            moles            moles            moles             moles             moles             moles            moles        '
  initial_temperature = 18.8
  temperature = 18.8
  kinetic_species_name = 'Basalt_SRP'
  kinetic_species_initial_value = '34.16500994'
  kinetic_species_unit = 'moles'
  evaluate_kinetic_rates_always = true # otherwise will easily 'run out' of dissolving species
  ramp_max_ionic_strength_initial = 0 # max_ionic_strength in such a simple problem does not need ramping
  stoichiometric_ionic_str_using_Cl_only = true # for comparison with GWB
  add_aux_pH = false
  execute_console_output_on = ''
  mol_cutoff = 1E-13
[]

[Executioner]
  type = Transient
  [TimeSteppers]
    [timesequence]
      type = CSVTimeSequenceStepper
      file_name = time.csv
      column_index = 0
    []
  []
  end_time = 31557600 #31557600 #1.9769E+04 #31557600
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
  [rxn_rate]
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
