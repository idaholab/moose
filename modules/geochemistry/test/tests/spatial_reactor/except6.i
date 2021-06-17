# exception testing: incorrect sizing of controlled_activity_name and controlled_activity_time
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
    controlled_activity_value = '1E-4 1'
[]

[Mesh]
  type = GeneratedMesh
  dim = 1
[]

[Executioner]
  type = Transient
  num_steps = 1
[]

