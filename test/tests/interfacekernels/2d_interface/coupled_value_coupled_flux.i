[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 2
    xmax = 2
    ny = 2
    ymax = 2
  []
  [./subdomain1]
    input = gen
    type = SubdomainBoundingBoxGenerator
    bottom_left = '0 0 0'
    top_right = '1 1 0'
    block_id = 1
  [../]
  [./interface]
    type = SideSetsBetweenSubdomainsGenerator
    input = subdomain1
    primary_block = '0'
    paired_block = '1'
    new_boundary = 'primary0_interface'
  [../]
  [./break_boundary]
    input = interface
    type = BreakBoundaryOnSubdomainGenerator
  [../]
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
    block = 0
  [../]

  [./v]
    order = FIRST
    family = LAGRANGE
    block = 1
  [../]
[]

[Kernels]
  [./diff_u]
    type = CoeffParamDiffusion
    variable = u
    D = 4
    block = 0
  [../]
  [./diff_v]
    type = CoeffParamDiffusion
    variable = v
    D = 2
    block = 1
  [../]
  [./source_u]
    type = BodyForce
    variable = u
    value = 1
  [../]
[]

[InterfaceKernels]
  [./interface]
    type = PenaltyInterfaceDiffusion
    variable = u
    neighbor_var = v
    boundary = primary0_interface
    penalty = 1e6
  [../]
[]

[BCs]
  [./u]
    type = VacuumBC
    variable = u
    boundary = 'left_to_0 bottom_to_0 right top'
  [../]
  [./v]
    type = VacuumBC
    variable = v
    boundary = 'left_to_1 bottom_to_1'
  [../]
[]

[Postprocessors]
  [./u_int]
    type = ElementIntegralVariablePostprocessor
    variable = u
    block = 0
  [../]
  [./v_int]
    type = ElementIntegralVariablePostprocessor
    variable = v
    block = 1
  [../]
[]

[Preconditioning]
  [./smp]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
[]

[Outputs]
  exodus = true
  print_linear_residuals = true
[]
