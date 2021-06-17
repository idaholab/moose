# demonstrating that controlled-activity can be spatially-dependent
# Here we output to the console at point (0.5, 0, 0)
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
    constraint_species = "H2O              H+            Cl-"
    constraint_value = "  1.0              -5            1E-5"
    constraint_meaning = "bulk_composition log10activity bulk_composition"
    constraint_unit = "   kg               dimensionless moles"
    controlled_activity_name = 'H+'
    controlled_activity_value = 'act_H+'
    execute_console_output_on = 'initial timestep_end'
    point = '0.8 0 0'
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
