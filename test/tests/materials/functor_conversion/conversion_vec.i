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
    family = LAGRANGE_VEC
    initial_condition = '2 2 2'
  []
[]

[AuxVariables]
  [v]
    order = FIRST
    family = MONOMIAL_VEC
    initial_condition = '3 3 3'
  []
[]

[FunctorMaterials]
  [block0]
    type = GenericVectorFunctorMaterial
    block = '0'
    prop_names = 'D'
    prop_values = '4 3 2'
  []
  [block1]
    type = GenericVectorFunctorMaterial
    block = '1'
    prop_names = 'D'
    prop_values = '2 1 0'
  []
[]

[Materials]
  [convert_to_reg]
    type = VectorMaterialFunctorConverter
    functors_in = 'D u v'
    reg_props_out = 'm1 m3 m4'
    outputs = 'exo'
  []
  [convert_to_ad]
    type = VectorMaterialFunctorConverter
    functors_in = 'D u v'
    ad_props_out = 'm1a m3a m4a'
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
