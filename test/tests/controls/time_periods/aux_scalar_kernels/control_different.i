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
  [./aux0]
    family = SCALAR
  [../]
  [./aux1]
    family = SCALAR
  [../]
[]

[Functions]
  [./func]
    type = ParsedFunction
    expression = t
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

[AuxScalarKernels]
  [./scalar_aux0]
    type = FunctionScalarAux
    variable = aux0
    function = func
  [../]
  [./scalar_aux1]
    type = FunctionScalarAux
    variable = aux1
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
  num_steps = 10
  dt = 0.1
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  csv = true
[]

[Controls]
  [./damping_control]
    type = TimePeriod
    disable_objects = 'AuxScalarKernels/scalar_aux0 */scalar_aux1'
    start_time      = '0.25 0.45'
    end_time        = '0.55 0.75'
    execute_on = 'initial timestep_begin'
  [../]
[]
