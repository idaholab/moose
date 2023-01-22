[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[Variables]
  [./u]
  [../]
  [./v]
  [../]
[]

[AuxVariables]
  [./exact_solution]
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

[NodalKernels]
  [./td]
    type = TimeDerivativeNodalKernel
    variable = v
  [../]
  [./f]
    type = UserForcingFunctionNodalKernel
    variable = v
    function = t*t*t+4
  [../]
[]

[AuxKernels]
  [./exact]
    type = FunctionAux
    variable = exact_solution
    function = exact_solution_function
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

[Functions]
  [./exact_solution_function]
    type = ParsedFunction
    expression = (1.0/4.0)*(16*t+t*t*t*t)
  [../]
[]

[Postprocessors]
  [./error]
    type = NodalL2Error
    variable = v
    function = exact_solution_function
  [../]
[]

[Executioner]
  type = Transient
  end_time = 10
  dt = 1
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
  scheme = 'crank-nicolson'
[]

[Outputs]
  exodus = true
[]
