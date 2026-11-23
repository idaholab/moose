# The spatial reactor may be restricted to a subset of the mesh using "block".  The AuxVariables
# and AuxKernels that the action adds are restricted to the same blocks, since the reactor only
# knows about the nodes it visits.
[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 10
    xmax = 1
  []
  [reactive]
    type = SubdomainBoundingBoxGenerator
    input = gen
    block_id = 1
    bottom_left = '0 0 0'
    top_right = '0.5 0 0'
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
  constraint_value = "  1                -5            1E-5"
  constraint_meaning = "bulk_composition log10activity bulk_composition"
  constraint_unit = "   kg               dimensionless moles"
  block = 1
[]

[Executioner]
  type = Transient
  num_steps = 1
[]

[Postprocessors]
  [pH_inside]
    type = PointValue
    point = '0.25 0 0'
    variable = pH
  []
  [pH_outside]
    # pH is restricted to block 1, the reactor's block, so it has no value here and this reads zero
    type = PointValue
    point = '0.75 0 0'
    variable = pH
  []
[]

[Outputs]
  csv = true
[]
