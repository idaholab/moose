#Progressively remove H2O until virtually none remains
[UserObjects]
  [definition]
    type = GeochemicalModelDefinition
    database_file = "../../../database/moose_geochemdb.json"
    basis_species = "H2O H+ Cl- Ca++ Mg++ Na+ K+ SO4-- HCO3-"
    equilibrium_minerals = "Dolomite Epsomite Gypsum Halite Magnesite Mirabilite Sylvite"
    equilibrium_gases = "CO2(g)"
    piecewise_linear_interpolation = true # for precise agreement with GWB
  []
[]

[TimeDependentReactionSolver]
  model_definition = definition
  geochemistry_reactor_name = reactor
  swap_out_of_basis = "H+"
  swap_into_basis = "  CO2(g)"
  charge_balance_species = "Cl-" # this means the bulk moles of Cl- will not be exactly as set below
  constraint_species = "H2O              CO2(g)        Cl-              Na+              SO4--            Mg++             Ca++             K+               HCO3-"
  constraint_value = "  1.0              -3.5          0.5656           0.4850           0.02924          0.05501          0.01063          0.010576055      0.002412"
  constraint_meaning = "kg_solvent_water log10fugacity bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition"
  constraint_unit = "   kg               dimensionless moles            moles            moles            moles            moles            moles            moles"
  close_system_at_time = 0
  source_species_names = "H2O"
  source_species_rates = "-1.0" # 1kg H2O = 55.51 moles, each time step removes 1 mole
  mode = mode
  ramp_max_ionic_strength_initial = 0 # not needed in this simple example
  stoichiometric_ionic_str_using_Cl_only = true # for precise agreement with GWB
  execute_console_output_on = '' # only CSV output for this example
[]

[Functions]
  [timestepper]
    type = PiecewiseLinear
    x = '0 50 55'
    y = '5 5 1'
  []
[]

[Executioner]
  type = Transient
  [TimeStepper]
    type = FunctionDT
    function = timestepper
  []
  end_time = 55
[]

[AuxVariables]
  [mode]
  []
[]

[AuxKernels]
  [mode]
    type = FunctionAux
    variable = mode
    function = 'if(t<=1.0, 1.0, 0.0)' # initial "dump" then "normal"
    execute_on = 'timestep_begin'
  []
[]

[GlobalParams]
  point = '0 0 0'
[]
[Postprocessors]
  [solvent_kg]
    type = PointValue
    variable = 'kg_solvent_H2O'
  []
  [dolomite]
    type = PointValue
    variable = 'free_cm3_Dolomite'
  []
  [gypsum]
    type = PointValue
    variable = 'free_cm3_Gypsum'
  []
  [halite]
    type = PointValue
    variable = 'free_cm3_Halite'
  []
  [mirabilite]
    type = PointValue
    variable = 'free_cm3_Mirabilite'
  []
[]

[Outputs]
  csv = true
[]
