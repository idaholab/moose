[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[Variables]
  [./u]
  [../]
[]

[Functions]
  [./diff_func]
    type = ParsedFunction
    expression = 1/t
  [../]
[]

[Kernels]
  [./diff]
    type = GenericDiffusion
    variable = u
    property = diffusion
  [../]
  [./td]
    type = TimeDerivative
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
  [./gfm]
    type = GenericFunctionMaterial
    block = 0
    prop_names = diffusion
    prop_values = diff_func
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 10
  dt = 0.1

  solve_type = 'PJFNK'

  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
[]
