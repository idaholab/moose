[Mesh]
  [msh]
  type = GeneratedMeshGenerator
  dim = 2
  nx = 50
  ny = 50
  nz = 0
  xmax = 40
  ymax = 40
  zmax = 0
  elem_type = QUAD4
  []
  [subdomain_1]
    type = SubdomainBoundingBoxGenerator
    input = msh
    bottom_left = '0 0 0'
    top_right = '5 7 0'
    block_id = 1
  []
  [subdomain_2]
    type = SubdomainBoundingBoxGenerator
    input = subdomain_1
    bottom_left = '0 7 0'
    top_right = '5 40 0'
    block_id = 2
  []
  [subdomain_3]
    type = SubdomainBoundingBoxGenerator
    input = subdomain_2
    bottom_left = '5 0 0'
    top_right = '40 7 0'
    block_id = 3
  []
  [subdomain_4]
    type = SubdomainBoundingBoxGenerator
    input = subdomain_3
    bottom_left = '5 7 0'
    top_right = '40 40 0'
    block_id = 4
  []
  [./split]
    type = BreakMeshByBlockGenerator
    input = subdomain_4
  []
[]

[Variables]
  active = 'u'

  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[AuxVariables]
  [./periodic_dist]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  active = 'diff forcing dot'

  [./diff]
    type = Diffusion
    variable = u
  [../]

  [./forcing]
    type = GaussContForcing
    variable = u
  [../]

  [./dot]
    type = TimeDerivative
    variable = u
  [../]
[]

[InterfaceKernels]
  [./interface]
    type = PenaltyInterfaceDiffusion
    variable = u
    neighbor_var = u
    boundary = interface
    penalty = 1e7
  [../]
[]

[AuxKernels]
  [./periodic_dist]
    type = PeriodicDistanceAux
    variable = periodic_dist
    point = '4 6 0'
  [../]
[]

[BCs]
  [./Periodic]
    [./all]
      variable = u
      auto_direction = 'x y'
    [../]
  [../]
[]

[Executioner]
  type = Transient
  dt = 1
  num_steps = 10
  solve_type = NEWTON
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_package'
  petsc_options_value = 'lu superlu_dist'
[]

[Outputs]
  execute_on = 'timestep_end'
  # file_base = out_auto
  exodus = true
[]
