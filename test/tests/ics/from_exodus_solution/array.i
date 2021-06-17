[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 5
    ny = 5
  []
[]

[Variables]
  [u]
    order = FIRST
    family = LAGRANGE
  []

  [v]
    order = CONSTANT
    family = MONOMIAL
    components = 2
  []
[]

[Kernels]
  [u_time]
    type = TimeDerivative
    variable = u
  []
  [u_diff]
    type = Diffusion
    variable = u
  []

  [v_time]
    type = ArrayTimeDerivative
    variable = v
    time_derivative_coefficient = tc
  []
  [v_reaction]
    type = ArrayCoupledForce
    variable = v
    v = u
    coef = '1 2'
  []
[]

[Materials/tc]
  type = GenericConstantArray
  prop_name = tc
  prop_value = '2 3'
[]

[BCs]
  [left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 1
  []
  [right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 2
  []
[]

[Executioner]
  type = Transient
  num_steps = 2
[]

[Outputs]
  exodus = true
[]
