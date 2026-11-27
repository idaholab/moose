# Debugging technique: the host and Kokkos versions of the Exercise 1 problem in separate solver
# systems of one run, with a postprocessor reporting their difference
[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 50
    ny = 50
  []
[]

[Problem]
  nl_sys_names = 'host kokkos'
[]

[Variables]
  [T_host]
    solver_sys = host
    initial_condition = 300
  []
  [T_kokkos]
    solver_sys = kokkos
    initial_condition = 300
  []
[]

[Kernels]
  [time_host]
    type = TimeDerivative
    variable = T_host
  []
  [diff_host]
    type = Diffusion
    variable = T_host
  []
  [source_host]
    type = BodyForce
    variable = T_host
    value = 1e3
  []
  [time_kokkos]
    type = KokkosTimeDerivative
    variable = T_kokkos
  []
  [diff_kokkos]
    type = KokkosDiffusion
    variable = T_kokkos
  []
  [source_kokkos]
    type = KokkosBodyForce
    variable = T_kokkos
    value = 1e3
  []
[]

[BCs]
  [left_host]
    type = DirichletBC
    variable = T_host
    boundary = left
    value = 300
  []
  [right_host]
    type = NeumannBC
    variable = T_host
    boundary = right
    value = -10
  []
  [left_kokkos]
    type = KokkosDirichletBC
    variable = T_kokkos
    boundary = left
    value = 300
  []
  [right_kokkos]
    type = KokkosNeumannBC
    variable = T_kokkos
    boundary = right
    value = -10
  []
[]

[Postprocessors]
  [T_host_avg]
    type = ElementAverageValue
    variable = T_host
  []
  [T_kokkos_avg]
    type = KokkosElementAverageValue
    variable = T_kokkos
  []
  [difference]
    type = ParsedPostprocessor
    expression = 'abs(T_kokkos_avg - T_host_avg)'
    pp_names = 'T_host_avg T_kokkos_avg'
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  dt = 0.1
  num_steps = 5
[]

[Outputs]
  csv = true
[]
