[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 2
[]

[Variables]
  [./u]
  [../]
[]

[Kernels]
  [./diff]
    type = MatDiffusion
    variable = u
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
    type = FunctionDirichletBC
    variable = u
    boundary = right
    function = constant
  [../]
[]

[Functions]
  [constant]
    type = ParsedFunction
    value = '1'
  []
  [ramp]
    type = ParsedFunction
    value = 't'
  []
[]


[Materials]
  active = 'constant'
  [constant]
    type = GenericConstantMaterial
    prop_names = 'D'
    prop_values = '1e20'
  []
  [function]
    type = GenericFunctionMaterial
    prop_names = 'D'
    prop_values = '10^(t-1)'
  []
[]

[Executioner]
  type = Transient
  num_steps = 1
  dt = 2
  dtmin = 2
  solve_type = NEWTON
  petsc_options = '-pc_svd_monitor -ksp_view_pmat -snes_converged_reason -ksp_converged_reason'
  petsc_options_iname = '-pc_type -snes_stol'
  petsc_options_value = 'svd      0'
[]

[Outputs]
  exodus = true
[]
