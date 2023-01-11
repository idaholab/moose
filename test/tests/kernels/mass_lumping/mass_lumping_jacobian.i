[Mesh]
  type = GeneratedMesh
  dim = 1
  xmin = -1
  xmax = 1
  nx = 2
[]

[Variables]
  [./u]
  [../]
[]

[ICs]
  [./u_init]
    type = FunctionIC
    variable = u
    function = init_f
  [../]
[]

[Kernels]
  [./time_deriv]
    type = MassLumpedTimeDerivative
    variable = u
  [../]
  [./diff]
    type = FuncCoefDiffusion
    variable = u
    coef = diff_f
  [../]
[]

[Functions]
  [./init_f]
    type = ParsedFunction
    expression = max(x,0) #(x>0)
  [../]
  [./diff_f]
    type = ParsedFunction
    expression = max(x,0)
  [../]
[]


[Executioner]
  type = Transient
  end_time = 1
  solve_type = 'NEWTON'
  petsc_options_iname = '-snes_type'
  petsc_options_value = 'test'
[]

[Outputs]
  exodus = true
[]
