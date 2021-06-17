# demonstrating that sources may be spatially-dependent, but adding sources of fixed-activity species makes no difference to the system
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
    constraint_meaning = "bulk_composition activity bulk_composition"
    constraint_unit = "moles dimensionless moles"
    remove_fixed_activity_name = 'H+'
    remove_fixed_activity_time = '2'
    source_species_names = HCl
    source_species_rates = HCl_rate
[]

[VectorPostprocessors]
  [bulk_Cl]
    type = LineValueSampler
    start_point = '0 0 0'
    end_point = '1 0 0'
    sort_by = x
    num_points = 11
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
    function = '1E-5 * x'
    execute_on = timestep_begin # so the Reactor gets the correct value
  []
[]

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 10
  xmax = 1
[]

[Executioner]
  type = Transient
  dt = 1
  end_time = 2
[]

[Outputs]
  csv = true
[]

