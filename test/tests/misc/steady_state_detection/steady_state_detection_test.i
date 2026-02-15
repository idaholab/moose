[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
[]

[Variables]
  [u]
  []
[]

[ICs]
  [u_ic]
    type = ConstantIC
    variable = u
    value = 1
  []
[]

[Kernels]
  [td]
    type = TimeDerivative
    variable = u
  []
  [diff]
    type = Diffusion
    variable = u
  []
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
    value = 1
  []
[]

[AuxVariables]
  [dummy]
    family = LAGRANGE
    order = FIRST
  []
[]

[AuxKernels]
  [dummy_aux]
    type = ParsedAux
    variable = dummy
    expression = 0.0
  []
[]

[Postprocessors]
  [dummy_avg]
    type = ElementAverageValue
    variable = dummy
  []
[]

[Executioner]
  type = Transient
  dt = 1
  num_steps = 10
  steady_state_detection = true
  check_aux = true
  steady_state_start_time = 1.5
  steady_state_tolerance = 1e-12
[]

[Outputs]
  csv = true
[]
