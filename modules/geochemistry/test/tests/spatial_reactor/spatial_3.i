# demonstrating that adding sources of a fixed-activity species makes no difference before the system is closed
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
    constraint_meaning = "bulk_composition free_concentration bulk_composition"
    constraint_unit = "moles molal moles"
    close_system_at_time = 3
    source_species_names = 'HCl'
    source_species_rates = '1.0'
[]

[Postprocessors]
  [pH]
    type = PointValue
    point = '0 0 0'
    variable = pH
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
  end_time = 2
[]

[Outputs]
  csv = true
[]

