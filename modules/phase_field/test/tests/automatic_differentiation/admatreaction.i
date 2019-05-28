[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 20
[]

[Variables]
  [./u]
    [./InitialCondition]
      type = FunctionIC
      function = cos(x*2*pi)
    [../]
  [../]
  [./v]
    [./InitialCondition]
      type = FunctionIC
      function = sin(x*2*pi)
    [../]
  [../]
[]

[Kernels]
  [./dudt]
    type = ADTimeDerivative
    variable = u
  [../]
  [./dvdt]
    type = ADTimeDerivative
    variable = v
  [../]
  [./u]
    type = ADMatReaction
    variable = u
    #v = v
  [../]
  [./v]
    type = ADMatReaction
    variable = v
  [../]
[]

[Materials]
  [./L]
    type = ADTestDerivativeFunction
    function = F3
    f_name = L
    op = 'u v'
  [../]
[]

[Preconditioning]
  [./smp]
    type = SMP
    full = true
  [../]
[]

[Debug]
  show_var_residual_norms = true
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  dt = 0.1
  num_steps = 5
[]

[Outputs]
  exodus = true
[]
