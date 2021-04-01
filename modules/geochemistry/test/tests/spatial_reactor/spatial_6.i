# demonstrating that temperature may be spatially-dependent
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
    temperature = temp_controller
[]

[VectorPostprocessors]
  [temperature]
    type = LineValueSampler
    start_point = '0 0 0'
    end_point = '10 0 0'
    sort_by = x
    num_points = 11
    variable = 'solution_temperature'
  []
[]

[AuxVariables]
  [temp_controller]
  []
[]

[AuxKernels]
  [temp_controller]
    type = FunctionAux
    variable = temp_controller
    function = '25 + x'
    execute_on = timestep_begin # so the Reactor gets the correct value
  []
[]

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 10
  xmax = 10
[]

[Executioner]
  type = Transient
  dt = 1
  end_time = 2
[]

[Outputs]
  csv = true
[]

