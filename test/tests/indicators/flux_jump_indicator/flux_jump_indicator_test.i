[Mesh]
  file = two_block_square.e
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  active = 'diff'
  [./diff]
    type = Diffusion
    variable = u
  [../]
  [./conv]
    type = Convection
    variable = u
    velocity = '1 0 0'
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  [../]
  [./right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  [../]
[]

[Materials]
  [./left]
    type = GenericConstantMaterial
    block = left
    prop_names = diffusivity
    prop_values = 1
  [../]
  [./right]
    type = GenericConstantMaterial
    block = right
    prop_names = diffusivity
    prop_values = 3
  [../]
[]

[Adaptivity]
  [./Indicators]
    [./error]
      type = FluxJumpIndicator
      variable = u
      property = diffusivity
    [../]
  [../]
[]

[Executioner]
  type = Steady

  # Preconditioned JFNK (default)
  solve_type = 'PJFNK'
[]

[Output]
  linear_residuals = true
  output_initial = true
  exodus = true
  perf_log = true
[]

