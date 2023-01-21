[Mesh]
  type = GeneratedMesh
  nx = 2
  dim = 1
[]

[Kernels]
  [./source]
    type = BodyForce
    variable = u
    function = 'forcing_func'
  [../]
  [./convection]
    type = ConservativeAdvection
    variable = u
    velocity = '1 0 0'
  [../]
  [./diffusion]
    type = MatDiffusionTest
    variable = u
    prop_name = 'k'
  [../]
[]

[DGKernels]
  [./convection]
    type = DGConvection
    variable = u
    velocity = '1 0 0'
  [../]
  [./diffusion]
    type = DGDiffusion
    variable = u
    diff = 'k'
    sigma = 6
    epsilon = -1
  [../]
[]

[BCs]
  [./advection]
    type = OutflowBC
    boundary = 'right'
    variable = u
    velocity = '1 0 0'
  [../]
  [./diffusion_left]
    type = DGFunctionDiffusionDirichletBC
    boundary = 'left'
    variable = u
    sigma = 6
    epsilon = -1
    function = 'boundary_left_func'
    diff = 'k'
  [../]
[]

[Variables]
  [./u]
    family = MONOMIAL
    order = THIRD
  [../]
[]

[Materials]
  [./test]
    block = 0
    type = GenericFunctionMaterial
    prop_names = 'k'
    prop_values = 'k_func'
  [../]
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
[]

[Preconditioning]
  [./SMP]
    type = SMP
    full = true
  [../]
[]

[Functions]
  [./forcing_func]
    type = ParsedFunction
    expression = '1'
  [../]
  [./boundary_left_func]
    type = ParsedFunction
    expression = '0'
  [../]
  [./k_func]
    type = ParsedFunction
    expression = '1 + x'
  [../]
[]

[Outputs]
  exodus = true
  execute_on = 'timestep_end'
[]
