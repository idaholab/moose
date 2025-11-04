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
    type = SideSetsBetweenSubdomainsGenerator
    input = 'subdomain1'
    primary_block = '0'
    paired_block = '1'
    new_boundary = 'primary0_interface'
  []
[]

[Variables]
  [u]
    order = FIRST
    family = LAGRANGE
    block = '0'
  []
[]

[AuxVariables]
  [v]
    order = FIRST
    family = LAGRANGE
    block = '1'
    initial_condition = 4
  []
[]

[Kernels]
  [diff_u]
    type = MatDiffusion
    variable = u
    block = '0'
    diffusivity = D
  []
[]

[InterfaceKernels]
  [interface]
    type = PenaltyInterfaceDiffusion
    variable = u
    neighbor_var = 'v'
    boundary = 'primary0_interface'
    # D = D
    # D_neighbor = D
    penalty = 1e3
  []
[]

[Materials]
  [block0]
    type = GenericConstantMaterial
    block = '0'
    prop_names = 'D'
    prop_values = '4'
  []
  [block1]
    type = GenericConstantMaterial
    block = '1'
    prop_names = 'D'
    prop_values = '2'
  []
[]

[Executioner]
  type = Steady
  solve_type = PJFNK
  nl_rel_tol = 1e-10
  nl_forced_its = 2
[]

[Problem]
  kernel_coverage_check = false
[]

[Outputs]
  csv = true
[]

[Postprocessors]
  [min]
    type = ElementExtremeValue
    variable = 'u'
    value_type = 'min'
    block = '0'
  []
  [max]
    type = ElementExtremeValue
    variable = 'u'
    block = '0'
  []
[]
