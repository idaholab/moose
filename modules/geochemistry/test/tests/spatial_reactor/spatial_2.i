# demonstrating that controlled-activity can be spatially-dependent and that adding sources of such species leaves the system unchanged
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
    controlled_activity_name = 'H+'
    controlled_activity_value = 'act_H+'
    source_species_names = 'H+'
    source_species_rates = '1.0'
[]

[VectorPostprocessors]
  [pH]
    type = LineValueSampler
    start_point = '0 0 0'
    end_point = '1 0 0'
    sort_by = x
    num_points = 11
    variable = pH
  []
[]

[AuxVariables]
  [act_H+]
  []
[]

[AuxKernels]
  [act_H+]
    type = FunctionAux
    variable = 'act_H+'
    function = '10^(-5 + x)'
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
  num_steps = 1
[]

[Outputs]
  csv = true
  execute_on = final
[]

