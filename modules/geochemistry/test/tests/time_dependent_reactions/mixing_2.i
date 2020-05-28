#The fluid from mixing_1.i at temperature=4degC is slowly mixed with this fluid initially at temperature=273degC until a 10:1 ratio is achieved
[TimeDependentReactionSolver]
  model_definition = definition
  geochemistry_reactor_name = reactor
  swap_into_basis = "H2S(aq)"
  swap_out_of_basis = "O2(aq)"
  charge_balance_species = "Cl-"
  constraint_species = "H2O H+     Cl-    Na+    Mg++    SO4--   Ca++    K+      HCO3-  Ba++  SiO2(aq) Sr++     Zn++  Cu+     Al+++  Fe++   Mn++    H2S(aq)"
  constraint_value = "1.0 6.31E-5  600E-3 529E-3 0.01E-6 0.01E-6 21.6E-3 26.7E-3 2.0E-3 15E-6 20.2E-3  100.5E-6 41E-6 0.02E-6 4.1E-6 903E-6 1039E-6 6.81E-3"
  constraint_meaning = "kg_solvent_water activity moles_bulk_species moles_bulk_species moles_bulk_species moles_bulk_species moles_bulk_species moles_bulk_species moles_bulk_species moles_bulk_species moles_bulk_species moles_bulk_species moles_bulk_species moles_bulk_species moles_bulk_species moles_bulk_species moles_bulk_species moles_bulk_species"
  close_system_at_time = -1.0
  remove_fixed_activity_name = 'H+'
  remove_fixed_activity_time = -1
  initial_temperature = 273
  temperature = T
  source_species_names = "Al+++ Ba++ Cl- Cu+ Fe++ H+ H2O HCO3- K+ Mg++ Mn++ Na+ O2(aq) SO4-- SiO2(aq) Sr++ Zn++"
  source_species_rates = "Al+++_rate Ba++_rate Cl-_rate Cu+_rate Fe++_rate H+_rate H2O_rate HCO3-_rate K+_rate Mg++_rate Mn++_rate Na+_rate O2aq_rate SO4--_rate SiO2aq_rate Sr++_rate Zn++_rate"
  mode = mode
  execute_console_output_on = 'final'
[]

[AuxVariables]
  [./T]
  [../]
  [./mode]
  [../]
  [./Al+++_rate]
  [../]
  [./Ba++_rate]
  [../]
  [./Cl-_rate]
  [../]
  [./Cu+_rate]
  [../]
  [./Fe++_rate]
  [../]
  [./H+_rate]
  [../]
  [./H2O_rate]
  [../]
  [./HCO3-_rate]
  [../]
  [./K+_rate]
  [../]
  [./Mg++_rate]
  [../]
  [./Mn++_rate]
  [../]
  [./Na+_rate]
  [../]
  [./O2aq_rate]
  [../]
  [./SO4--_rate]
  [../]
  [./SiO2aq_rate]
  [../]
  [./Sr++_rate]
  [../]
  [./Zn++_rate]
  [../]
  []
[AuxKernels]
  [./mode]
    type = FunctionAux
    variable = mode
    function = 'if(t<=0, 1, 0)' # dump at start of first timestep
    execute_on = timestep_begin
  [../]
  [./T]
    type = FunctionAux
    variable = T
    function = 'if(t<=0, 273, 4)' # during initialisation and dumping, T=273, while during adding T=temperature of reactants
    execute_on = timestep_begin
  [../]
  [./Al+++_rate]
    type = FunctionAux
    variable = Al+++_rate
    execute_on = timestep_begin
    function = 'if(t<=0, 0, 5e-09 )'
  [../]
  [./Ba++_rate]
    type = FunctionAux
    variable = Ba++_rate
    execute_on = timestep_begin
    function = 'if(t<=0, 0, 2e-07 )'
  [../]
  [./Cl-_rate]
    type = FunctionAux
    variable = Cl-_rate
    execute_on = timestep_begin
    function = 'if(t<=0, 0, 0.55874102284486 )'
  [../]
  [./Cu+_rate]
    type = FunctionAux
    variable = Cu+_rate
    execute_on = timestep_begin
    function = 'if(t<=0, 0, 7e-09 )'
  [../]
  [./Fe++_rate]
    type = FunctionAux
    variable = Fe++_rate
    execute_on = timestep_begin
    function = 'if(t<=0, 0, 1e-09 )'
  [../]
  [./H+_rate]
    type = FunctionAux
    variable = H+_rate
    execute_on = timestep_begin
    function = 'if(t<=0, 0, -0.0001394231551447 )'
  [../]
  [./H2O_rate]
    type = FunctionAux
    variable = H2O_rate
    execute_on = timestep_begin
    function = 'if(t<=0, 0, 55.510029367259 )'
  [../]
  [./HCO3-_rate]
    type = FunctionAux
    variable = HCO3-_rate
    execute_on = timestep_begin
    function = 'if(t<=0, 0, 0.0024 )'
  [../]
  [./K+_rate]
    type = FunctionAux
    variable = K+_rate
    execute_on = timestep_begin
    function = 'if(t<=0, 0, 0.0101 )'
  [../]
  [./Mg++_rate]
    type = FunctionAux
    variable = Mg++_rate
    execute_on = timestep_begin
    function = 'if(t<=0, 0, 0.0545 )'
  [../]
  [./Mn++_rate]
    type = FunctionAux
    variable = Mn++_rate
    execute_on = timestep_begin
    function = 'if(t<=0, 0, 1e-09 )'
  [../]
  [./Na+_rate]
    type = FunctionAux
    variable = Na+_rate
    execute_on = timestep_begin
    function = 'if(t<=0, 0, 0.48 )'
  [../]
  [./O2aq_rate]
    type = FunctionAux
    variable = O2aq_rate
    execute_on = timestep_begin
    function = 'if(t<=0, 0, 0.00012300205331609 )'
  [../]
  [./SO4--_rate]
    type = FunctionAux
    variable = SO4--_rate
    execute_on = timestep_begin
    function = 'if(t<=0, 0, 0.0295 )'
  [../]
  [./SiO2aq_rate]
    type = FunctionAux
    variable = SiO2aq_rate
    execute_on = timestep_begin
    function = 'if(t<=0, 0, 0.00017 )'
  [../]
  [./Sr++_rate]
    type = FunctionAux
    variable = Sr++_rate
    execute_on = timestep_begin
    function = 'if(t<=0, 0, 9e-05 )'
  [../]
  [./Zn++_rate]
    type = FunctionAux
    variable = Zn++_rate
    execute_on = timestep_begin
    function = 'if(t<=0, 0, 1e-08)'
  [../]
[]

[Postprocessors]
  [./temperature]
    type = PointValue
    point = '0 0 0'
    variable = "solution_temperature"
  [../]
  [./fugactity_O2]
    type = PointValue
    point = '0 0 0'
    variable = "activity_O2(g)"
  [../]
  [./molal_SO4--]
    type = PointValue
    point = '0 0 0'
    variable = "molal_SO4--"
  [../]
  [./molal_NaSO4]
    type = PointValue
    point = '0 0 0'
    variable = "molal_NaSO4-"
  [../]
  [./molal_H2Saq]
    type = PointValue
    point = '0 0 0'
    variable = "molal_H2S(aq)"
  [../]
  [./molal_HSO4-]
    type = PointValue
    point = '0 0 0'
    variable = "molal_HSO4-"
  [../]
  [./cm3_Anhydrite]
    type = PointValue
    point = '0 0 0'
    variable = "free_cm3_Anhydrite"
  [../]
  [./cm3_Pyrite]
    type = PointValue
    point = '0 0 0'
    variable = "free_cm3_Pyrite"
  [../]
  [./cm3_Talc]
    type = PointValue
    point = '0 0 0'
    variable = "free_cm3_Talc"
  [../]
  [./cm3_AmSil]
    type = PointValue
    point = '0 0 0'
    variable = "free_cm3_Amrph^silica"
  [../]
[]

[Executioner]
  type = Transient
  start_time = -1 # to allow initial dump to occur
  dt = 1
  end_time = 10
[]

[UserObjects]
  [./definition]
    type = GeochemicalModelDefinition
    database_file = "../../../database/moose_geochemdb.json"
    basis_species = "H2O H+ Cl- Na+ Mg++ SO4-- Ca++ K+ HCO3- Ba++ SiO2(aq) Sr++ Zn++ Cu+ Al+++ Fe++ Mn++ O2(aq)"
    equilibrium_minerals = "Anhydrite Pyrite Talc Amrph^silica"
    equilibrium_gases = "O2(g)"
  [../]
[]

