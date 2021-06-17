# exception testing: unknown source_species
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
    source_species_names = 'Ca++'
    source_species_rates = '1'
[]

[Mesh]
  type = GeneratedMesh
  dim = 1
[]

[Executioner]
  type = Transient
  num_steps = 1
[]

