# exception testing: attempt to remove a fixed activity from a secondary species
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
    remove_fixed_activity_name = 'OH-'
    remove_fixed_activity_time = 1E-4
[]

[Mesh]
  type = GeneratedMesh
  dim = 1
[]

[Executioner]
  type = Transient
  num_steps = 1
[]

