# A block-restricted reactor knows only about the nodes in its blocks, so an AuxKernel that queries
# it elsewhere is an error.  The AuxKernels added by the action are restricted automatically; this
# one is written by hand without a block restriction.
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

[AuxVariables]
  [unrestricted]
  []
[]

[AuxKernels]
  [unrestricted]
    type = GeochemistryQuantityAux
    variable = unrestricted
    species = H2O
    quantity = molal
    reactor = geochemistry_reactor
    execute_on = timestep_end
  []
[]

[Executioner]
  type = Transient
  num_steps = 1
[]
