# Navier-Stokes (or Euler) flow of an ideal gas over a step.
#
# Note: this problem is not currently a regression test for the
# Navier-Stokes module since it is in some sense ill-posed.  As
# discussed in [0], the sharp corner of the step (both forward and
# backward-facing) introduces a singularity in the first derivative of
# the velocity and pressure fields, and therefore produces large
# numerical errors in the neighborhood of these points.  Physically,
# this numerical error can be interpreted as causing an artificial
# "boundary layer" to form just above the step, as well as a spurious
# production of entropy even though the flow remains subsonic.
# Nevertheless, the forward-facing step problem in particular remains
# a challenging and well-document test problem for flow solvers, and
# this input file is included to help facilitate its development and
# employment by users of the module.
#
# [0]: Woodward and Colella, "The numerical simulation of
# two-dimenstional fluid flow with strong shocks," Journal of
# Computational Physics 54(1), pp. 115-173, 1984

[Mesh]
  type = FileMesh
  file = step.e
  dim = 2
  # uniform_refine = 3
[]



[FluidProperties]
  [ideal_gas]
    type = IdealGasFluidProperties
    gamma = 1.4
  []
[]



[Modules]
  [CompressibleNavierStokes]
    # steady-state or transient
    equation_type = transient

    # fluid
    fluid_properties = ideal_gas

    # boundary conditions
    stagnation_boundary = left
    stagnation_pressure = 120192.995549849 # Pa, Mach=0.5 at 1 atm
    stagnation_temperature = 315 # K, Mach=0.5 at 1 atm
    stagnation_flow_direction = '1 0'
    no_penetration_boundary = 'top bottom step_top step_left step_right'
    static_pressure_boundary = 'right'
    static_pressure = 101325 # Pa

    # variable types, scalings and initial conditions
    family = LAGRANGE
    order = FIRST
    total_energy_scaling = 9.869232667160121e-6
    initial_pressure = 101325.
    initial_temperature = 300.
    initial_velocity = '173.594354746921 0 0' # Mach 0.5: = 0.5*sqrt(gamma*R*T)
  []
[]



[Materials]
  [fluid]
    type = Air
    block = 1
    rho = rho
    rhou = rhou
    rhov = rhov
    rhoE = rhoE
    vel_x = vel_x
    vel_y = vel_y
    temperature = temperature
    enthalpy = enthalpy
    # This value is not used in the Euler equations, but it *is* used
    # by the stabilization parameter computation, which it decreases
    # the amount of artificial viscosity added, so it's best to use a
    # realistic value.
    dynamic_viscosity = 0.0
    fluid_properties = ideal_gas
  []
[]



[Preconditioning]
  [SMP_PJFNK]
    type = SMP
    full = true
  []
[]



[Executioner]
  type = Transient
  dt = 5.e-5
  dtmin = 1.e-5
  start_time = 0.0
  num_steps = 10000
  nl_rel_tol = 1e-5
  nl_abs_tol = 1e-9
  # nl_abs_step_tol = 1e-15
  nl_max_its = 5
  l_tol = 1e-4 # Relative linear tolerance for each Krylov solve
  l_max_its = 100 # Number of linear iterations for each Krylov solve

  # Specify the order as FIRST, otherwise you will get warnings in DEBUG mode...
  [Quadrature]
    type = TRAP
    order = FIRST
  []
[]



[Outputs]
  file_base = step_out
  interval = 1
  exodus = true
[]
