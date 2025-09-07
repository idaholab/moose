# Domain on (0,2) with 2 equally sized elements.
# Source point is at p = (1.1,0,0), in second element.
#
# Two variables: u (FE), v (FV)
#
#   du/dt = dirac(p) v,   u(0) = 0
#   dv/dt = 3,            v(0) = 2
#
# Taking 3 time steps of 1 second each. Values and Volume integrals should be
#
#   t = 0: v = 2,  (u) = 0
#   t = 1: v = 5,  (u) = 5
#   t = 2: v = 8,  (u) = 13
#   t = 3: v = 11, (u) = 24

source_point = '1.1 0 0'

u0 = 0
v0 = 2

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 2
    xmin = 0
    xmax = 2
  []
[]

[Variables]
  [u]
    family = LAGRANGE
    order = FIRST
    initial_condition = ${u0}
  []
  [v]
    type = MooseVariableFVReal
    initial_condition = ${v0}
  []
[]

[Kernels]
  [time_deriv_u]
    type = ADTimeDerivative
    variable = u
  []
[]

[FVKernels]
  [time_deriv_v]
    type = FVTimeKernel
    variable = v
  []
  [source_v]
    type = FVBodyForce
    variable = v
    function = 3
  []
[]

[DiracKernels]
  [source_u]
    type = FunctorDiracKernel
    variable = u
    functor = v
    point = ${source_point}
  []
[]

[Postprocessors]
  [u_integral]
    type = ElementIntegralVariablePostprocessor
    variable = u
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [v_avg]
    type = ElementAverageValue
    variable = v
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]

[Preconditioning]
  [pc]
    type = SMP
    full = true
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
