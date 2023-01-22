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
  [./parent_aux]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[Functions]
  [./func]
    type = ParsedFunction
    expression = x*y*t
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

[AuxKernels]
  [./func_aux]
    type = FunctionAux
    variable = parent_aux
    function = func
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
  num_steps = 5
  dt = 0.1
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
  nl_rel_tol = 1e-12
[]

[Outputs]
  exodus = true
[]

[MultiApps]
  [./quad]
    type = TransientMultiApp
    app_type = MooseTestApp
    positions = '0.05 0.05 0 0.95 0.05 0 0.05 0.95 0 0.95 0.95 0'
    input_files = quad_sub.i
  [../]
[]

[Transfers]
  [./parent_to_sub]
    type = MultiAppVariableValueSamplePostprocessorTransfer
    to_multi_app = quad
    source_variable = parent_aux
    postprocessor = pp
  [../]
[]
