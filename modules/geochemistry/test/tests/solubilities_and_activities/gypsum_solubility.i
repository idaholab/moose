[TimeDependentReactionSolver]
  model_definition = definition
  swap_out_of_basis = "Ca++"
  swap_into_basis = "Gypsum"
  charge_balance_species = "SO4--"
  constraint_species = "H2O              Cl-                Na+                SO4--            Gypsum"
  constraint_value = "  1.0              1E-10              1E-10              1E-6             0.5814"
  constraint_meaning = "kg_solvent_water free_concentration free_concentration bulk_composition free_mineral"
  constraint_unit = "   kg               molal              molal              moles            moles"
  source_species_names = 'NaCl'
  source_species_rates = '1.0'
  add_aux_pH = false # there is no H+ in the problem
  ramp_max_ionic_strength_initial = 0 # not needed in this simple problem
  stoichiometric_ionic_str_using_Cl_only = true # for comparison with GWB
  abs_tol = 1E-12
  execute_console_output_on = '' # only CSV output in this example
[]

[UserObjects]
  [definition]
    type = GeochemicalModelDefinition
    database_file = "../../../database/moose_geochemdb.json"
    basis_species = "H2O Cl- Na+ SO4-- Ca++"
    equilibrium_minerals = "Gypsum"
    piecewise_linear_interpolation = true # for comparison with GWB
  []
[]

[Functions]
  [timestepper]
    type = PiecewiseLinear
    x = '0    0.1'
    y = '0.01 0.1'
  []
[]

[Executioner]
  type = Transient
  [TimeStepper]
    type = FunctionDT
    function = timestepper
  []
  end_time = 3
[]

[Outputs]
  csv = true
[]

[AuxVariables]
  [dissolved_gypsum_moles]
  []
[]
[AuxKernels]
  [dissolved_gypsum_moles]
    type = ParsedAux
    coupled_variables = 'bulk_moles_Gypsum free_mg_Gypsum'
    expression = 'bulk_moles_Gypsum - free_mg_Gypsum / 1000 / 172.168 '
    variable = dissolved_gypsum_moles
    execute_on = 'timestep_end'
  []
[]

[Postprocessors]
  [cl_molal]
    type = PointValue
    point = '0 0 0'
    variable = 'molal_Cl-'
  []
  [dissolved_gypsum_mol]
    type = PointValue
    point = '0 0 0'
    variable = dissolved_gypsum_moles
  []
[]
