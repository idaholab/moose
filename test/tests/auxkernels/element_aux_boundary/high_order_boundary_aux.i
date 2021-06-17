[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 4
    ny = 4
  []
[]

[Variables]
  [u]
  []
[]

[AuxVariables]
  [real_property]
    family = MONOMIAL
    order = SECOND
  []
[]

[AuxKernels]
  [real_property]
    type = MaterialRealAux
    variable = real_property
    property = real_property
    boundary = '0 2'
  []
[]

[Kernels]
  [diff]
    type = CoefDiffusion
    variable = u
    coef = 1
  []
  [time]
    type = TimeDerivative
    variable = u
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = u
    boundary = 0
    value = 3
  []
  [right]
    type = DirichletBC
    variable = u
    boundary = 2
    value = 1
  []
[]

[Materials]
  [boundary_mat]
    type = OutputTestMaterial
    boundary = '0 1 2 3'
    real_factor = 2
    variable = u
  []
[]

[Executioner]
  type = Transient
  num_steps = 2
  dt = 0.1
[]

[Outputs]
  exodus = true
[]
