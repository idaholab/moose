[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 2
  ny = 2
  elem_type = QUAD4
  uniform_refine = 4
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  [./diff]
    type = CoefDiffusion
    variable = u
    coef = 0.1
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

[Executioner]
  type = Transient
  num_steps = 3
  dt = 0.1
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[DiracKernels]
  [./test_object]
    type = MaterialPointSource
    point = '0.5 0.5 0'
    variable = u
  [../]
[]

[Materials]
  [./mat]
    type = GenericConstantMaterial
    prop_names = 'matp'
    prop_values = '1'
    block = 0
  [../]
[]

[Postprocessors]
  [./test_object]
    type = FunctionValuePostprocessor
    function = '2*(x+y)'
    point = '0.5 0.5 0'
  [../]
  [./other_point_test_object]
    type = FunctionValuePostprocessor
    function = '3*(x+y)'
    point = '0.5 0.5 0'
  [../]
[]

[Outputs]
  controls = true
[]

[Controls]
  [./point_control]
    type = TestControl
    test_type = 'point'
    parameter = '*/*/point'
    execute_on = 'initial'
  [../]
[]
