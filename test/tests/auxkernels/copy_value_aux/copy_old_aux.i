[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 1
  ny = 1
[]

[Variables]
  [T]
  []
[]

[AuxVariables]
  [T_old]
  []
  [T_older]
  []
[]

[Kernels]
  [hctd]
    type = TimeDerivative
    variable = T
  []
  [hs]
    type = BodyForce
    variable = T
    value = 1
  []
[]

[AuxKernels]
  [T_old]
    type = CopyValueAux
    variable = T_old
    source = T
    state = OLD
    execute_on = 'initial timestep_end'
  []
  [T_older]
    type = CopyValueAux
    variable = T_older
    source = T
    state = OLDER
    execute_on = 'initial timestep_end'
  []
[]

[ICs]
  [T]
    type = ConstantIC
    variable = T
    value = 0
  []
[]

[Postprocessors]
  [T]
    type = ElementAverageValue
    variable = T
    execute_on = 'timestep_end'
  []
  [T_old]
    type = ElementAverageValue
    variable = T_old
    execute_on = 'timestep_end'
  []
  [T_older]
    type = ElementAverageValue
    variable = T_older
    execute_on = 'timestep_end'
  []
[]

[Executioner]
  type = Transient
  [TimeIntegrator]
    type = ActuallyExplicitEuler
    solve_type = lumped
  []
  solve_type = LINEAR
  num_steps = 3
[]

[Outputs]
  csv = true
[]
