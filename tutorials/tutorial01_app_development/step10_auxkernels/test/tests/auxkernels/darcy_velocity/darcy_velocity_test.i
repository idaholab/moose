[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[Variables]
  [pressure]
    order = FIRST
    family = LAGRANGE
  []
[]

[AuxVariables]
  [velocity]
    order = CONSTANT
    family = MONOMIAL_VEC
  []
[]

[Kernels]
  [diffusion]
    type = DarcyPressure
    variable = pressure
  []
[]

[AuxKernels]
  [velocity]
    type = DarcyVelocity
    variable = velocity
    pressure = pressure
    execute_on = TIMESTEP_END
  []
[]

[Materials]
  [column]
    type = PackedColumn
  []
[]

[BCs]
  [left]
    type = ADDirichletBC
    variable = pressure
    boundary = left
    value = 0
  []
  [right]
    type = ADDirichletBC
    variable = pressure
    boundary = right
    value = 1
  []
[]

[Executioner]
  type = Steady
  solve_type = PJFNK
  l_tol = 1e-07 # tighter tolerance to acheive a numerically constant velocity field on 64-bit procs
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
[]
