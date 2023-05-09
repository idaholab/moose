[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmax = 2
    nx = 10
    ymax = 2
    ny = 10
  []
  [subdomain1]
    input = gen
    type = SubdomainBoundingBoxGenerator
    bottom_left = '0 0 0'
    block_id = 1
    top_right = '1 1 0'
  []
  [interface]
    input = subdomain1
    type = SideSetsBetweenSubdomainsGenerator
    primary_block = '1'
    paired_block = '0'
    new_boundary = 'primary1_interface'
  []
  [boundaries]
    input = interface
    type = BreakBoundaryOnSubdomainGenerator
    boundaries = 'left bottom'
  []
[]

[Variables]
  [u]
    order = FIRST
    family = L2_LAGRANGE
    block = 1
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
  [source]
    type = BodyForce
    variable = u
  []
[]

[DGKernels]
  [dg_diffusion]
    type = DGDiffusion
    variable = u
    sigma = 4
    epsilon = 1
  []
[]

[BCs]
  [vacuum]
    type = VacuumBC
    variable = u
    boundary = 'left_to_1 bottom_to_1'
  []
  [primary1_inteface]
    type = VacuumBC
    variable = u
    boundary = 'primary1_interface'
  []
[]

[Postprocessors]
  [norm]
    type = ElementL2Norm
    variable = u
    block = 1
  []
[]

[Executioner]
  type = Steady
  nl_abs_tol = 1e-12
[]

[Problem]
  kernel_coverage_check = false
[]

[Outputs]
  exodus = true
[]
