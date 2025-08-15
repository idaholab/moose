# Domain on (0,1) with 1 element
#
#   du/dt = v,   u(0) = 0
#   dv/dt = 1,   v(0) = 0
#
# Taking 3 time steps of 1 second each. Values should be the following:
#
#   t = 0: u = 0, v = 0
#   t = 1: u = 1, v = 1
#   t = 2: u = 3, v = 2
#   t = 3: u = 6, v = 3

u0 = 0
v0 = 0

v_source = 1

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 1
    xmin = 0
    xmax = 1
  []
[]

[Variables]
  [u]
    family = LAGRANGE
    order = FIRST
    initial_condition = ${u0}
  []
  [v]
    family = SCALAR
    order = FIRST
    initial_condition = ${v0}
  []
[]

[Kernels]
  [time_deriv_u]
    type = ADTimeDerivative
    variable = u
  []
  [source_u]
    type = FunctorKernel
    variable = u
    functor = source_u
    functor_on_rhs = true
  []
[]

[FunctorMaterials]
  [u_source_mat]
    type = ADParsedFunctorMaterial
    expression = 'v'
    functor_names = 'v'
    property_name = source_u
  []
[]

[ScalarKernels]
  [time_deriv_v]
    type = ADScalarTimeDerivative
    variable = v
  []
  [source_v]
    type = ParsedODEKernel
    variable = v
    expression = '-${v_source}'
  []
[]

[Postprocessors]
  [u_avg]
    type = ElementAverageValue
    variable = u
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]

[Executioner]
  type = Transient
  dt = 1
  num_steps = 3
  solve_type = NEWTON
[]

[Outputs]
  csv = true
[]
