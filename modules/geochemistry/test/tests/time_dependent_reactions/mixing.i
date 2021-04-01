# Seawater at temperature=4degC is slowly mixed with this fluid initially at temperature=273degC until a 10:1 ratio is achieved
[TimeDependentReactionSolver]
  model_definition = definition
  geochemistry_reactor_name = reactor
  swap_into_basis = "H2S(aq)"
  swap_out_of_basis = "O2(aq)"
  charge_balance_species = "Cl-"
  constraint_species = "H2O              H+            Cl-              Na+              Mg++             SO4--            Ca++             K+               HCO3-            Ba++            SiO2(aq)          Sr++             Zn++             Cu+              Al+++            Fe++             Mn++             H2S(aq)"
  constraint_value = "  1.0              6.309573E-5   600E-3           529E-3           0.01E-6          0.01E-6          21.6E-3          26.7E-3          2.0E-3           15E-6           20.2E-3           100.5E-6         41E-6            0.02E-6          4.1E-6           903E-6           1039E-6          6.81E-3"
  constraint_meaning = "kg_solvent_water activity      bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition"
  constraint_unit = "   kg               dimensionless moles            moles            moles            moles            moles            moles            moles            moles            moles            moles            moles            moles            moles            moles            moles            moles"
  close_system_at_time = -0.01
  remove_fixed_activity_name = 'H+'
  remove_fixed_activity_time = -0.01
  initial_temperature = 273
  temperature = T
  # The following source species and rates are taken from the Geochemists Workbench (see output from mixing.rea)
  # An alternative is to run the seawater_mixing MOOSE input files and extract the source species and rates
  source_species_names = "H2O Al+++ Ba++ Ca++ Cl- Cu+ Fe++ H+ HCO3- K+ Mg++ Mn++ Na+ O2(aq) SO4-- SiO2(aq) Sr++ Zn++"
  source_species_rates = "H2O_rate Al+++_rate Ba++_rate Ca++_rate Cl-_rate Cu+_rate Fe++_rate H+_rate HCO3-_rate K+_rate Mg++_rate Mn++_rate Na+_rate O2aq_rate SO4--_rate SiO2aq_rate Sr++_rate Zn++_rate"
  mode = mode
  execute_console_output_on = '' # only CSV output needed for this example
  stoichiometric_ionic_str_using_Cl_only = true # for comparison with GWB
[]

[AuxVariables]
  [T]
  []
  [mode]
  []
  [H2O_rate]
  []
  [Al+++_rate]
  []
  [Ba++_rate]
  []
  [Ca++_rate]
  []
  [Cl-_rate]
  []
  [Cu+_rate]
  []
  [Fe++_rate]
  []
  [H+_rate]
  []
  [HCO3-_rate]
  []
  [K+_rate]
  []
  [Mg++_rate]
  []
  [Mn++_rate]
  []
  [Na+_rate]
  []
  [O2aq_rate]
  []
  [SO4--_rate]
  []
  [SiO2aq_rate]
  []
  [Sr++_rate]
  []
  [Zn++_rate]
  []
[]
[AuxKernels]
  [mode_auxk]
    type = FunctionAux
    variable = mode
    function = 'if(t<=0, 1, 0)' # dump at start of first timestep
    execute_on = timestep_begin
  []
  [T_auxk]
    type = FunctionAux
    variable = T
    function = 'if(t<=0, 273, 4)' # during initialisation and dumping, T=273, while during adding T=temperature of reactants
    execute_on = timestep_begin
  []
  [H2O_rate_auxk]
    type = FunctionAux
    variable = H2O_rate
    execute_on = timestep_begin
    function = 'if(t<=0, 0, 55.510000000000005)'
  []
  [Al+++_rate]
    type = FunctionAux
    variable = Al+++_rate
    execute_on = timestep_begin
    function = 'if(t<=0, 0, 3.643e-10)'
  []
  [Ba++_rate]
    type = FunctionAux
    variable = Ba++_rate
    execute_on = timestep_begin
    function = 'if(t<=0, 0, 8.831e-08)'
  []
  [Ca++_rate]
    type = FunctionAux
    variable = Ca++_rate
    execute_on = timestep_begin
    function = 'if(t<=0, 0, 0.0104)'
  []
  [Cl-_rate]
    type = FunctionAux
    variable = Cl-_rate
    execute_on = timestep_begin
    function = 'if(t<=0, 0, 0.559)'
  []
  [Cu+_rate]
    type = FunctionAux
    variable = Cu+_rate
    execute_on = timestep_begin
    function = 'if(t<=0, 0, 7.000000000000001e-09)'
  []
  [Fe++_rate]
    type = FunctionAux
    variable = Fe++_rate
    execute_on = timestep_begin
    function = 'if(t<=0, 0, 4.746e-15)'
  []
  [H+_rate]
    type = FunctionAux
    variable = H+_rate
    execute_on = timestep_begin
    function = 'if(t<=0, 0, 0.0002005)'
  []
  [HCO3-_rate]
    type = FunctionAux
    variable = HCO3-_rate
    execute_on = timestep_begin
    function = 'if(t<=0, 0, 0.002153)'
  []
  [K+_rate]
    type = FunctionAux
    variable = K+_rate
    execute_on = timestep_begin
    function = 'if(t<=0, 0, 0.010100000000000001)'
  []
  [Mg++_rate]
    type = FunctionAux
    variable = Mg++_rate
    execute_on = timestep_begin
    function = 'if(t<=0, 0, 0.054400000000000004)'
  []
  [Mn++_rate]
    type = FunctionAux
    variable = Mn++_rate
    execute_on = timestep_begin
    function = 'if(t<=0, 0, 6.79e-14)'
  []
  [Na+_rate]
    type = FunctionAux
    variable = Na+_rate
    execute_on = timestep_begin
    function = 'if(t<=0, 0, 0.48019999999999996)'
  []
  [O2aq_rate]
    type = FunctionAux
    variable = O2aq_rate
    execute_on = timestep_begin
    function = 'if(t<=0, 0, 0.000123)'
  []
  [SO4--_rate]
    type = FunctionAux
    variable = SO4--_rate
    execute_on = timestep_begin
    function = 'if(t<=0, 0, 0.0295)'
  []
  [SiO2aq_rate]
    type = FunctionAux
    variable = SiO2aq_rate
    execute_on = timestep_begin
    function = 'if(t<=0, 0, 0.00017)'
  []
  [Sr++_rate]
    type = FunctionAux
    variable = Sr++_rate
    execute_on = timestep_begin
    function = 'if(t<=0, 0, 3.8350000000000004e-05)'
  []
  [Zn++_rate]
    type = FunctionAux
    variable = Zn++_rate
    execute_on = timestep_begin
    function = 'if(t<=0, 0, 1e-08)'
  []
[]

[Postprocessors]
  [temperature]
    type = PointValue
    point = '0 0 0'
    variable = "solution_temperature"
  []
  [fugactity_O2]
    type = PointValue
    point = '0 0 0'
    variable = "activity_O2(g)"
  []
  [molal_SO4--]
    type = PointValue
    point = '0 0 0'
    variable = "molal_SO4--"
  []
  [molal_NaSO4]
    type = PointValue
    point = '0 0 0'
    variable = "molal_NaSO4-"
  []
  [molal_H2Saq]
    type = PointValue
    point = '0 0 0'
    variable = "molal_H2S(aq)"
  []
  [molal_HSO4-]
    type = PointValue
    point = '0 0 0'
    variable = "molal_HSO4-"
  []
  [cm3_Anhydrite]
    type = PointValue
    point = '0 0 0'
    variable = "free_cm3_Anhydrite"
  []
  [cm3_Pyrite]
    type = PointValue
    point = '0 0 0'
    variable = "free_cm3_Pyrite"
  []
  [cm3_Talc]
    type = PointValue
    point = '0 0 0'
    variable = "free_cm3_Talc"
  []
  [cm3_AmSil]
    type = PointValue
    point = '0 0 0'
    variable = "free_cm3_Amrph^silica"
  []
[]

[Functions]
  [timestepper]
    type = PiecewiseLinear
    x = '0    0.1  1   10'
    y = '0.01 0.01 0.5 10'
  []
[]

[Executioner]
  type = Transient
  start_time = -0.01 # to allow initial dump to occur
  [TimeStepper]
    type = FunctionDT
    function = timestepper
  []
  end_time = 10
[]

[UserObjects]
  [definition]
    type = GeochemicalModelDefinition
    database_file = "../../../database/moose_geochemdb.json"
    basis_species = "H2O H+ Cl- Na+ Mg++ SO4-- Ca++ K+ HCO3- Ba++ SiO2(aq) Sr++ Zn++ Cu+ Al+++ Fe++ Mn++ O2(aq)"
    equilibrium_minerals = "Anhydrite Pyrite Talc Amrph^silica Barite Dolomite-ord Muscovite Nontronit-Na Pyrolusite Strontianite"
    equilibrium_gases = "O2(g)"
  []
[]

[Outputs]
  csv = true
[]
