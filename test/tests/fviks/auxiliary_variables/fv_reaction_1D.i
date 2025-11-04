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
    type = MooseVariableFVReal
    block = '0'
  []
[]

[AuxVariables]
  [v]
    type = MooseVariableFVReal
    block = '1'
    initial_condition = 4
  []
[]

[FVKernels]
  [diff_u]
    type = FVDiffusion
    variable = u
    block = '0'
    coeff = 1
  []
[]

[FVInterfaceKernels]
  [interface]
    type = FVDiffusionInterface
    variable1 = u
    variable2 = 'v'
    boundary = 'primary0_interface'
    coeff1 = 1
    coeff2= 2
    subdomain1 = 0
    subdomain2 = 1
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
