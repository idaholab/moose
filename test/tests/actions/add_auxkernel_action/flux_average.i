[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[Variables]
  [u]
  []
[]

[AuxVariables]
  [flux]
    order = CONSTANT
    family = MONOMIAL
    [AuxKernel]
      type = FluxAverageAux
      coupled = u
      diffusivity = 0.1
      boundary = right
    []
  []
[]

[Functions]
  [bc_func]
    type = ParsedFunction
    expression = y+1
  []
[]

[Kernels]
  [diff]
    type = CoefDiffusion
    variable = u
    coef = 0.1
  []
  [time]
    type = TimeDerivative
    variable = u
  []
[]

[AuxKernels]
  [flux_average]
    type = FluxAverageAux
    variable = flux
    coupled = u
    diffusivity = 0.1
    boundary = right
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  []
  [right]
    type = FunctionDirichletBC
    variable = u
    boundary = right
    function = bc_func
  []
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
  exodus = true
[]
