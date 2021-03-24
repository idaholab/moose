# Sources are spatially-dependent and adaptive timestepping is needed to ensure convergence
[UserObjects]
  [definition]
    type = GeochemicalModelDefinition
    database_file = "../../../database/moose_geochemdb.json"
    basis_species = "H2O H+ Cl-"
  []
[]

[SpatialReactionSolver]
    model_definition = definition
    charge_balance_species = "Cl-"
    constraint_species = "H2O H+ Cl-"
    constraint_value = "  55.5 1E-5 1E-5"
    constraint_meaning = "bulk_composition bulk_composition bulk_composition"
    constraint_unit = "moles moles moles"
    source_species_names = HCl
    source_species_rates = HCl_rate
    execute_console_output_on = ''
    ramp_max_ionic_strength_initial = 0
    max_iter = 2
    adaptive_timestepping = true
[]

[VectorPostprocessors]
  [bulk_Cl]
    type = LineValueSampler
    start_point = '0 0 0'
    end_point = '1 0 0'
    sort_by = x
    num_points = 2
    variable = 'bulk_moles_Cl-'
  []
[]

[AuxVariables]
  [HCl_rate]
  []
[]

[AuxKernels]
  [HCl_rate]
    type = FunctionAux
    variable = HCl_rate
    function = '1 * x'
    execute_on = timestep_begin # so the Reactor gets the correct value
  []
[]

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
  xmax = 1
[]

[Executioner]
  type = Transient
  dt = 1
  end_time = 1
[]

[Outputs]
  csv = true
[]

