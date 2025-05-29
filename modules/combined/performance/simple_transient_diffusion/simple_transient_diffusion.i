[Mesh/gmg]
  type = DistributedRectilinearMeshGenerator
  dim = 3
  nx = 100
  ny = 100
  nz = 100
[]

[Variables/u]
  order = FIRST
  family = LAGRANGE
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
  [source]
    type = BodyForce
    variable = u
    value = 1
  []
[]

[Postprocessors/average]
  type = ElementAverageValue
  variable = u
[]

[BCs/all]
  type = DirichletBC
  variable = u
  boundary = 'top right bottom left front back'
  value = 0
[]

[Executioner]
  type = Transient
  num_steps = 10
  dt = 0.1
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'hypre'
  solve_type = 'NEWTON'
[]

[Outputs]
  csv = true
[]
