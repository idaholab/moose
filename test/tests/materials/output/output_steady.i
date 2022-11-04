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
  [./bc_func]
    type = ParsedFunction
    expression = 0.5*y
  [../]
[]

[Kernels]
  [./diff]
    type = CoefDiffusion
    variable = u
    block = 0
    coef = 0.1
  [../]
[]

[BCs]
  [./left]
    type = FunctionDirichletBC
    variable = u
    boundary = left
    function = bc_func
  [../]
  [./right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  [../]
[]

[Materials]
  [./k]
    type = OutputTestMaterial
    block = 0
    outputs = all
    variable = u
    output_properties = 'real_property vector_property tensor_property'
  [../]
[]

[Executioner]
  type = Steady
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
[]
