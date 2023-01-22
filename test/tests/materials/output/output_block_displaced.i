[Mesh]
  type = FileMesh
  file = rectangle.e
  dim = 2
  uniform_refine = 1
  displacements = 'disp disp'
[]

[Functions]
  [./disp_fn]
    type = ParsedFunction
    expression = x
  [../]
[]

[AuxVariables]
  [./disp]
  [../]
[]

[AuxKernels]
  [./disp_ak]
    type = FunctionAux
    variable = disp
    function = disp_fn
  [../]
[]

[Variables]
  [./u]
  [../]
[]

[Kernels]
  [./diff]
    type = CoefDiffusion
    variable = u
    coef = 0.5
  [../]
  [./time]
    type = TimeDerivative
    variable = u
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    boundary = 1
    value = 0
  [../]
  [./right]
    type = DirichletBC
    variable = u
    boundary = 2
    value = 2
  [../]
[]

[Materials]
  [./block_1]
    type = OutputTestMaterial
    block = 1
    output_properties = 'real_property tensor_property'
    outputs = exodus
    variable = u
  [../]
  [./block_2]
    type = OutputTestMaterial
    block = 2
    output_properties = 'vector_property tensor_property'
    outputs = exodus
    variable = u
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 5
  dt = 0.1
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  [./exodus]
    type = Exodus
    use_displaced = true
    sequence = false
  [../]
[]
