# Euler flow of an ideal gas over a Gaussian "bump".
#
# The inlet is a stagnation pressure and temperature BC which
# corresponds to subsonic (M=0.5) flow with a static pressure of 1 atm
# and static temperature of 300K.  The outlet consists of a
# weakly-imposed static pressure BC of 1 atm.  The top and bottom
# walls of the channel weakly impose the "no normal flow" BC. The
# problem is initialized with freestream flow throughout the domain.
# Although this initial condition is less physically realistic, it
# helps the problem reach steady state more quickly.
#
# There is a sequence of uniformly-refined, geometry-fitted meshes
# from Yidong Xia available for solving this classical subsonic test
# problem (see the Mesh block below).  A coarse grid is used for the
# actual regression test, but changing one line in the Mesh block is
# sufficient to run this problem with different meshes.  An
# entropy-based error estimate is also provided, and can be used to
# demonstrate convergence of the numerical solution (since the true
# solution should produce zero entropy).  The error should converge at
# second-order in this norm.
[Mesh]
  # Bi-Linear elements
  # file = SmoothBump_quad_ref1_Q1.msh # 84 elems, 65 nodes
  # file = SmoothBump_quad_ref2_Q1.msh # 192 elems, 225 nodes
  # file = SmoothBump_quad_ref3_Q1.msh # 768 elems, 833 nodes
  # file = SmoothBump_quad_ref4_Q1.msh # 3072 elems, 3201 nodes
  # file = SmoothBump_quad_ref5_Q1.msh # 12288 elems, 12545 nodes
  # Bi-Quadratic elements
  # file = SmoothBump_quad_ref0_Q2.msh # 32 elems, 65 nodes
  # file = SmoothBump_quad_ref1_Q2.msh # 84 elems, 225 nodes
  file = SmoothBump_quad_ref2_Q2.msh # 260 elems, 833 nodes
  # file = SmoothBump_quad_ref3_Q2.msh # 900 elems, 3201 nodes
  # file = SmoothBump_quad_ref4_Q2.msh # 3332 elems, 12545 nodes
  # file = SmoothBump_quad_ref5_Q2.msh # 12804 elems, 49665 nodes
[]

[FluidProperties]
  [ideal_gas]
    type = IdealGasFluidProperties
    gamma = 1.4
    molar_mass = 0.02897024320557491
  []
[]

[Modules]
  [CompressibleNavierStokes]
    # steady-state or transient
    equation_type = transient

    # fluid
    fluid_properties = ideal_gas

    # boundary conditions
    stagnation_boundary = 1
    stagnation_pressure = 120192.995549849 # Pa, Mach=0.5 at 1 atm
    stagnation_temperature = 315 # K, Mach=0.5 at 1 atm
    stagnation_flow_direction = '1 0'
    no_penetration_boundary = '3 4'
    static_pressure_boundary = 2
    static_pressure = 101325 # Pa

    # variable types, scalings and initial conditions
    family = LAGRANGE
    order = FIRST
    total_energy_density_scaling = 9.869232667160121e-6
    initial_pressure = 101325.
    initial_temperature = 300.
    initial_velocity = '173.594354746921 0 0' # Mach 0.5: = 0.5*sqrt(gamma*R*T)
    pressure_variable_name = "p"
  []
[]

[Materials]
  [fluid]
    type = Air
    block = 0 # 'MeshInterior'
    rho = rho
    rhou = rhou
    rhov = rhov
    rho_et = rho_et
    vel_x = vel_x
    vel_y = vel_y
    temperature = temperature
    ht = ht
    # This value is not used in the Euler equations, but it *is* used
    # by the stabilization parameter computation, which it decreases
    # the amount of artificial viscosity added, so it's best to use a
    # realistic value.
    dynamic_viscosity = 0.0
    fluid_properties = ideal_gas
  []
[]

[Postprocessors]
  [entropy_error]
    type = NSEntropyError
    execute_on = 'initial timestep_end'
    block = 0
    rho_infty = 1.1768292682926829
    p_infty = 101325
    rho = rho
    pressure = p
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
  num_steps = 10
  nl_rel_tol = 1e-9
  nl_max_its = 5
  l_tol = 1e-4
  l_max_its = 100

  # We use trapezoidal quadrature.  This improves stability by
  # mimicking the "group variable" discretization approach.
  [Quadrature]
    type = TRAP
    order = FIRST
  []
[]

[Outputs]
  interval = 1
  exodus = true
[]

[AuxVariables]
  [rhoe][]
  [enthalpy][]
[]

[AuxKernels]
  [rhoe]
    variable = rhoe
    type = ParsedAux
    expression = 'rho_et'
    coupled_variables = 'rho_et'
    execute_on = 'initial timestep_end'
  []
  [enthalpy]
    variable = enthalpy
    type = ParsedAux
    expression = 'ht'
    coupled_variables = 'ht'
    execute_on = 'initial timestep_end'
  []
[]
