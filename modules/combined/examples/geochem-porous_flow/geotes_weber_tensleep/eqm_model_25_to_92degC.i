[UserObjects]
  [definition]
    type = GeochemicalModelDefinition
    database_file = "../../../../geochemistry/database/moose_geochemdb.json"
    basis_species = "H2O H+ Cl- SO4-- HCO3- SiO2(aq) Al+++ Ca++ Mg++ Fe++ K+ Na+ Sr++ F- B(OH)3 Br- Ba++ Li+ NO3- O2(aq)"
    equilibrium_minerals = "Siderite Pyrrhotite Dolomite Illite Anhydrite Calcite Quartz K-feldspar Kaolinite Barite Celestite Fluorite Albite Chalcedony Goethite"
  []
[]

[TimeDependentReactionSolver]
  model_definition = definition
  geochemistry_reactor_name = reactor
  swap_out_of_basis = "NO3- O2(aq)"
  swap_into_basis = "  NH3  HS-"
  charge_balance_species = "Cl-"
  constraint_species = "H2O H+          Cl-         SO4--       HCO3-       HS-         SiO2(aq)    Al+++       Ca++        Mg++        Fe++        K+          Na+         Sr++        F-         B(OH)3      Br-         Ba++        Li+         NH3"
  constraint_value = "  1.0 0.019675774 1.619044933 0.062774835 0.065489838 0.003840583 0.001597755 0.000129719 0.013448104 0.001851471 0.000787867 0.048851229 1.587660615 0.000159781 0.00032108 0.006663119 0.001238987 0.000101944 0.013110503 0.001937302"
  constraint_meaning = "kg_solvent_water bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition"
  constraint_unit = "kg moles moles moles moles moles moles moles moles moles moles moles moles moles moles moles moles moles moles moles"
  prevent_precipitation = "Celestite Fluorite Albite Chalcedony Goethite"
  ramp_max_ionic_strength_initial = 0 # not needed in this simple problem
  initial_temperature = 25
  temperature = 95 # so final temp = 92
  execute_console_output_on = 'initial timestep_end'
  source_species_names = "Siderite    Pyrrhotite  Dolomite    Illite      Anhydrite   Calcite    Quartz     K-feldspar  Kaolinite   Barite"
  source_species_rates = "6.287111422 0.510783201 2.796550921 0.647761624 1.175446234 12.1838956 322.504833 6.613392119 5.96865E-05 8.46449E-05"
  solver_info = true
  stoichiometric_ionic_str_using_Cl_only = true
[]

[Executioner]
  type = Transient
  dt = 1
  end_time = 1
[]

[AuxVariables]
  [total_mineral_volume]
  []
  [free_cm3_Kfeldspar] # necessary because of the minus sign in K-feldspar which does not parse correctly in the total_mineral_volume AuxKernel
  []
[]

[AuxKernels]
  [free_cm3_Kfeldspar]
    type = GeochemistryQuantityAux
    reactor = reactor
    variable = free_cm3_Kfeldspar
    species = 'K-feldspar'
    quantity = free_cm3
    execute_on = 'timestep_begin timestep_end'
  []
  [total_mineral_volume_auxk]
    type = ParsedAux
    coupled_variables = 'free_cm3_Siderite free_cm3_Pyrrhotite free_cm3_Dolomite free_cm3_Illite free_cm3_Anhydrite free_cm3_Calcite free_cm3_Quartz free_cm3_Kfeldspar free_cm3_Kaolinite free_cm3_Barite free_cm3_Celestite free_cm3_Fluorite free_cm3_Albite free_cm3_Chalcedony free_cm3_Goethite'
    expression = 'free_cm3_Siderite + free_cm3_Pyrrhotite + free_cm3_Dolomite + free_cm3_Illite + free_cm3_Anhydrite + free_cm3_Calcite + free_cm3_Quartz + free_cm3_Kfeldspar + free_cm3_Kaolinite + free_cm3_Barite + free_cm3_Celestite + free_cm3_Fluorite + free_cm3_Albite + free_cm3_Chalcedony + free_cm3_Goethite'
    variable = total_mineral_volume
    execute_on = 'timestep_begin timestep_end'
  []
[]

[Postprocessors]
  [total_mineral_volume]
    type = PointValue
    point = '0 0 0'
    variable = total_mineral_volume
  []
[]

[Outputs]
  csv = true
[]
