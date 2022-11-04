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

[AuxVariables]
  [./a]
    family = SCALAR
    order = FIRST
  [../]
[]

[AuxScalarKernels]
  [./a_sk]
    type = ConstantScalarAux
    variable = a
    value = 0
  [../]
[]

[Kernels]
  [./diff]
    type = CoefDiffusion
    variable = u
    coef = 0.1
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
  type = Steady
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  csv = true
[]

[Functions]
  [./func_coef]
    type = ParsedFunction
    expression = 1
  [../]
[]

[Controls]
  # We start with a = 0, control2 sets its value to 1 and then control1 will multiply it by 3,
  # so the end value has to be 3. If dependecy is broken, we multiply by 3 and then set to 1,
  # which is wrong
  [./control1]
    type = TestControl
    parameter = 'AuxScalarKernels/a_sk/value'
    test_type = MULT
    execute_on = 'initial timestep_begin'
    depends_on = control2
  [../]
  [./control2]
    type = RealFunctionControl
    parameter = 'AuxScalarKernels/a_sk/value'
    function = 'func_coef'
    execute_on = 'initial timestep_begin'
  [../]
[]
