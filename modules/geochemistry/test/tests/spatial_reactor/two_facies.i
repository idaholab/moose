# Two facies, each with its own block-restricted spatial reactor and its own block-restricted
# NodalVoidVolume.

# The mesh is four unit elements, x = 0 to 4, aquifer (block 1) covering x < 2 and seal (block 0)
# beyond it, with porosity 0.5 and 0.25 respectively.  With linear Lagrange each element gives
# porosity/2 to each of its nodes, so at the boundary node x = 2:
#
#   nvv_aquifer  0.5/2           = 0.25
#   nvv_seal            0.25/2   = 0.125
#   nvv_all      0.5/2 + 0.25/2  = 0.375

[Mesh]
  [gen]
    type = CartesianMeshGenerator
    dim = 1
    dx = '1 1 1 1'
  []
  [aquifer]
    type = SubdomainBoundingBoxGenerator
    input = gen
    block_id = 1
    bottom_left = '0 0 0'
    top_right = '2 0 0'
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Transient
  num_steps = 1
[]

[Outputs]
  csv = true
[]

[UserObjects]
  [definition]
    type = GeochemicalModelDefinition
    database_file = "../../../database/moose_geochemdb.json"
    basis_species = "H2O H+ Cl-"
  []
  [reactor_aquifer]
    type = GeochemistrySpatialReactor
    model_definition = definition
    charge_balance_species = "Cl-"
    constraint_species = "H2O H+ Cl-"
    constraint_value = "  1                -5            1E-5"
    constraint_meaning = "bulk_composition log10activity bulk_composition"
    constraint_unit = "   kg               dimensionless moles"
    block = 1
    execute_on = 'timestep_end'
  []
  [reactor_seal]
    type = GeochemistrySpatialReactor
    model_definition = definition
    charge_balance_species = "Cl-"
    constraint_species = "H2O H+ Cl-"
    constraint_value = "  1                -3            1E-5"
    constraint_meaning = "bulk_composition log10activity bulk_composition"
    constraint_unit = "   kg               dimensionless moles"
    block = 0
    execute_on = 'timestep_end'
  []
  [nvv_aquifer]
    type = NodalVoidVolume
    porosity = porosity
    block = 1
  []
  [nvv_seal]
    type = NodalVoidVolume
    porosity = porosity
    block = 0
  []
  [nvv_all]
    type = NodalVoidVolume
    porosity = porosity
  []
[]

[AuxVariables]
  [porosity]
    family = MONOMIAL
    order = CONSTANT
  []
  [pH_aquifer]
    block = 1
  []
  [pH_seal]
    block = 0
  []
  [vol_aquifer]
    block = 1
  []
  [vol_seal]
    block = 0
  []
  [vol_all]
  []
[]

[AuxKernels]
  [porosity]
    type = FunctionAux
    variable = porosity
    function = 'if(x<2, 0.5, 0.25)'
    execute_on = 'initial timestep_end'
  []
  [pH_aquifer]
    type = GeochemistryQuantityAux
    variable = pH_aquifer
    species = 'H+'
    quantity = neglog10a
    reactor = reactor_aquifer
    block = 1
    execute_on = 'timestep_end'
  []
  [pH_seal]
    type = GeochemistryQuantityAux
    variable = pH_seal
    species = 'H+'
    quantity = neglog10a
    reactor = reactor_seal
    block = 0
    execute_on = 'timestep_end'
  []
  [vol_aquifer]
    type = NodalVoidVolumeAux
    variable = vol_aquifer
    nodal_void_volume_uo = nvv_aquifer
    block = 1
  []
  [vol_seal]
    type = NodalVoidVolumeAux
    variable = vol_seal
    nodal_void_volume_uo = nvv_seal
    block = 0
  []
  [vol_all]
    type = NodalVoidVolumeAux
    variable = vol_all
    nodal_void_volume_uo = nvv_all
  []
[]

[Postprocessors]
  # the boundary node carries a chemistry from each facies
  [pH_aquifer_at_boundary]
    type = PointValue
    point = '2 0 0'
    variable = pH_aquifer
  []
  [pH_seal_at_boundary]
    type = PointValue
    point = '2 0 0'
    variable = pH_seal
  []
  # and the two void volumes partition that node exactly
  [vol_aquifer_at_boundary]
    type = PointValue
    point = '2 0 0'
    variable = vol_aquifer
  []
  [vol_seal_at_boundary]
    type = PointValue
    point = '2 0 0'
    variable = vol_seal
  []
  [vol_all_at_boundary]
    type = PointValue
    point = '2 0 0'
    variable = vol_all
  []
  [partition_residual]
    # zero if the two restricted void volumes partition the node's full void volume
    type = ParsedPostprocessor
    expression = 'aquifer + seal - all'
    pp_names = 'vol_aquifer_at_boundary vol_seal_at_boundary vol_all_at_boundary'
    pp_symbols = 'aquifer seal all'
  []
[]
