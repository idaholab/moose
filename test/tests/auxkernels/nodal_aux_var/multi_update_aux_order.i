[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = 0
  xmax = 1
  ymin = 0
  ymax = 1
  nx = 2
  ny = 1
[]

[Variables]
  [u]
    order = FIRST
    family = LAGRANGE
  []
[]

[AuxVariables]
  [tt]
    order = FIRST
    family = LAGRANGE
    initial_condition = 0
  []

  [ten]
    order = FIRST
    family = LAGRANGE
    initial_condition = 1
  []

  [2k]
    order = FIRST
    family = LAGRANGE
    initial_condition = 2
  []

  [test]
    order = FIRST
    family = LAGRANGE
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
  [dt]
    type = TimeDerivative
    variable = u
  []
[]

[AuxKernels]
  [test]
    # We comnstruct `test` befor `all`. Without correct dependency
    # resolution the `test` AuxKernel gets run before `all` updates
    # the value of `ten`, causing `test` to lag.
    type = ParsedAux
    variable = test
    expression = 'ten'
    coupled_variables = ten
    execute_on = 'TIMESTEP_END'
  []
  [all]
    variable = tt
    type = MultipleUpdateAux
    u = u
    var1 = ten
    var2 = 2k
    execute_on = 'TIMESTEP_END'
  []
[]

[BCs]
  active = 'left right'

  [left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  []

  [right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  []
[]

[Executioner]
  type = Transient
  num_steps = 3
  dt = 0.1
  solve_type = NEWTON
[]

[Outputs]
  exodus = true
[]
