# The reactor must time-step each node once, including the node at x = 1 that the two blocks
# share.  One mole of H2O is added per unit time to the initial 55.5 moles, so after one step of
# dt = 1 every node should hold 56.5 moles.  A node stepped once per block it belongs to would
# instead hold 57.5
[Mesh]
  [gen]
    type = CartesianMeshGenerator
    dim = 1
    dx = '1 1'
  []
  [left]
    type = SubdomainBoundingBoxGenerator
    input = gen
    block_id = 1
    bottom_left = '0 0 0'
    top_right = '1 0 0'
  []
[]

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
  source_species_names = 'H2O'
  source_species_rates = '1.0'
[]

[Executioner]
  type = Transient
  num_steps = 1
[]

[Postprocessors]
  [left]
    type = PointValue
    point = '0 0 0'
    variable = bulk_moles_H2O
  []
  [shared]
    type = PointValue
    point = '1 0 0'
    variable = bulk_moles_H2O
  []
  [right]
    type = PointValue
    point = '2 0 0'
    variable = bulk_moles_H2O
  []
[]

[Outputs]
  csv = true
[]
