[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 50
  ny = 50
[]

[Variables]
  [u]
    order = FIRST
    family = LAGRANGE
  []
[]

[AuxVariables]
  [u_aux]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[Distributions]
  [uniform]
    type = Uniform
    lower_bound = 1.0
    upper_bound = 3.0
  []
[]

[ICs]
  [u_aux]
    type = RandomIC
    legacy_generator = false
    variable = u_aux
    distribution = uniform
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = u
    boundary = 3
    value = 0
  []

  [right]
    type = DirichletBC
    variable = u
    boundary = 1
    value = 1
  []
[]

[VectorPostprocessors]
  [histo]
    type = VariableValueVolumeHistogram
    variable = u_aux
    min_value = 0
    max_value = 4
    bin_number = 80
    execute_on = initial
    outputs = initial
  []
[]

[Executioner]
  type = Steady
  solve_type = 'PJFNK'

  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  [initial]
    type = CSV
    execute_on = initial
  []
[]
