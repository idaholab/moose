[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 10
    xmax = 2
  []
  [subdomain1]
    input = gen
    type = SubdomainBoundingBoxGenerator
    bottom_left = '1.0 0 0'
    block_id = 1
    top_right = '2.0 1.0 0'
  []
  [interface]
    input = subdomain1
    type = SideSetsBetweenSubdomainsGenerator
    primary_block = '0'
    paired_block = '1'
    new_boundary = 'primary0_interface'
  []
[]

[Variables]
  [u]
    order = FIRST
    family = LAGRANGE
    initial_condition = 2
  []
[]

[AuxVariables]
  [v]
    order = FIRST
    family = MONOMIAL
    initial_condition = 3
  []
[]

[Functions]
  [f1]
    type = ParsedFunction
    expression = '2 + x'
  []
[]

[FunctorMaterials]
  [block0]
    type = GenericFunctorMaterial
    block = '0'
    prop_names = 'D D_block0'
    prop_values = '4 3'
  []
  [block1]
    type = GenericFunctorMaterial
    block = '1'
    prop_names = 'D'
    prop_values = '2'
  []
[]

[Materials]
  [convert_to_reg]
    type = MaterialFunctorConverter
    functors_in = 'D f1 u v'
    reg_props_out = 'm1 m2 m3 m4'
    outputs = 'exo'
  []
  [convert_to_ad]
    type = MaterialFunctorConverter
    functors_in = 'D f1 u v'
    ad_props_out = 'm1a m2a m3a m4a'
    outputs = 'exo'
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
[]

[Problem]
  solve = false
[]

[Outputs]
  [exo]
    type = Exodus
    hide = 'u v'
  []
[]
