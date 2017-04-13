[Mesh]
  type = GeneratedMesh
  dim = 2
  xmax = 2
  nx = 10
  ymax = 2
  ny = 10
  parallel_type = replicated
[]

[MeshModifiers]
  [./subdomain1]
    type = SubdomainBoundingBox
    bottom_left = '0 0 0'
    block_id = 1
    top_right = '1 1 0'
  [../]
  [./interface]
    type = SideSetsBetweenSubdomains
    depends_on = subdomain1
    master_block = '1'
    paired_block = '0'
    new_boundary = 'master1_interface'
  [../]
  [./boundaries]
    depends_on = interface
    type = BreakBoundaryOnSubdomain
    boundaries = 'left bottom'
  [../]
[]

[Problem]
  kernel_coverage_check = false
[]

[Variables]
  [./u]
    order = FIRST
    family = L2_LAGRANGE
    block = 1
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
    block = 1
  [../]
  [./source]
    type = BodyForce
    variable = u
    block = 1
  [../]
[]

[DGKernels]
  [./dg_diffusion]
    type = DGDiffusion
    variable = u
    sigma = 4
    epsilon = 1
    block = 1
  [../]
[]

[BCs]
  [./vacuum]
    type = VacuumBC
    variable = u
    boundary = 'left_to_1 bottom_to_1'
  [../]
  [./master1_inteface]
    type = VacuumBC
    variable = u
    boundary = 'master1_interface'
  [../]
[]

[Postprocessors]
  [./norm]
    type = ElementL2Norm
    variable = u
  [../]
[]

[Executioner]
  type = Steady
  nl_abs_tol = 1e-12
[]

[Outputs]
  exodus = true
[]
