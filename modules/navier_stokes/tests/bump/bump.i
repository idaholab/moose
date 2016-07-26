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
[GlobalParams]
  # Ratio of specific heats
  gamma = 1.4
  Pr = 0.71
  R = 287
[]



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



[Variables]
  [./rho]
    order = FIRST
    family = LAGRANGE

    [./InitialCondition]
      type = ConstantIC
      value = 1.17682926829268 # rho = P/RT = 101325.0 / 287 / 300, kg/m^3
    [../]
  [../]

  [./rhou]
    order = FIRST
    family = LAGRANGE

    # Comment out this section to start with zero initial flow
    [./InitialCondition]
      type = ConstantIC
      value = 204.290917476559 # Mach 0.5: rho * 0.5 * sqrt(gamma*R*T)
    [../]
  [../]

  [./rhov]
    order = FIRST
    family = LAGRANGE
  [../]

  [./rhoe]
    order = FIRST
    family = LAGRANGE

    # The magnitude of this equation is much larger than the others,
    # so we can scale it by the atmospheric value to get things closer
    # to the same size in the residual vector.
    scaling = ${/ 1. 101325.}

    # p=1 atm everywhere
    [./InitialCondition]
      # rho*E = rho*(e + (1/2)*u^2)
      #       = rho*(cv*T + (1/2)*u^2)
      #       = rho*((R/(gamma-1)*T + (1/2)*u^2)
      type = ConstantIC
      # Zero flow everywhere: 1.17682926829268*(287/.4*300)
      # value = 253312.5
      # Initial flow everywhere: 1.17682926829268*(287/.4*300 + 0.5*173.59435474692143**2)
      value = 271044.375
    [../]
  [../]
[]



[AuxVariables]
  [./vel_x]
    order = FIRST
    family = LAGRANGE

    # Comment out this section to start with zero initial flow
    [./InitialCondition]
      type = ConstantIC
      value = 173.594354746921 # Mach 0.5: = 0.5*sqrt(gamma*R*T)
    [../]
  [../]

  [./vel_y]
    order = FIRST
    family = LAGRANGE
  [../]

  [./Mach]
    order = FIRST
    family = LAGRANGE
  [../]

  [./temperature]
    order = FIRST
    family = LAGRANGE

    # Set constant temperature initial condition
    [./InitialCondition]
      type = ConstantIC

      # T = e_i / c_v
      #   = (e_t - 1/2*V^2) / c_v
      #   = (e_t) / c_v
      #   = rho*e_t / rho / c_v
      #   = P / (gamma-1) / rho / c_v
      value = 300 # K
    [../]
  [../]

  [./pressure]
    order = FIRST
    family = LAGRANGE

    [./InitialCondition]
      type = ConstantIC
      value = 101325 # Pa, 1 atm
    [../]
  [../]

  [./enthalpy]
    order = FIRST
    family = LAGRANGE
    # H = E + p/rho
    #   = (e + 1/2*|u|^2    ) + p/rho
    #   = (c_v*T + 1/2*|u|^2) + p/rho
    #   = (R/(gamma-1)*T + 1/2*|u|^2) + p/rho
    [./InitialCondition]
      type = ConstantIC
      # Zero flow everywhere: 287/.4*300 + 101325/1.17682926829268
      # value = 301350.0
      # Initial flow everywhere: 287/.4*300 + 0.5*173.59435474692143**2 + 101325/1.17682926829268
      value = 316417.5
    [../]
  [../]
[]



[Kernels]
  ################################################################################
  # Mass conservation Equation
  ################################################################################

  # Time derivative term
  [./rho_ie]
    type = TimeDerivative
    variable = rho
  [../]

  # Inviscid flux term (integrated by parts)
  [./rho_if]
    type = NSMassInviscidFlux
    variable = rho
    rho = rho
    rhou = rhou
    rhov = rhov
    rhoe = rhoe
    u = vel_x
    v = vel_y
  [../]

  ################################################################################
  # x-momentum equation
  ################################################################################

  # Time derivative term
  [./rhou_ie]
    type = TimeDerivative
    variable = rhou
  [../]

  # Inviscid flux term (integrated by parts)
  [./rhou_if]
    type = NSMomentumInviscidFlux
    variable = rhou
    u = vel_x
    v = vel_y
    rho = rho
    rhou = rhou
    rhov = rhov
    rhoe = rhoe
    pressure = pressure
    component = 0
  [../]

  ################################################################################
  # y-momentum equation
  ################################################################################

  # Time derivative term
  [./rhov_ie]
    type = TimeDerivative
    variable = rhov
  [../]

  # Inviscid flux term (integrated by parts)
  [./rhov_if]
    type = NSMomentumInviscidFlux
    variable = rhov
    u = vel_x
    v = vel_y
    rho = rho
    rhou = rhou
    rhov = rhov
    rhoe = rhoe
    pressure = pressure
    component = 1
  [../]


  ################################################################################
  # Total Energy Equation
  ################################################################################

  # Time derivative term
  [./rhoe_ie]
    type = TimeDerivative
    variable = rhoe
  [../]

  # Energy equation inviscid flux term (integrated by parts)
  [./rhoe_if]
    type = NSEnergyInviscidFlux
    variable = rhoe
    rho = rho
    rhou = rhou
    rhov = rhov
    rhoe = rhoe
    u = vel_x
    v = vel_y
    enthalpy = enthalpy
  [../]


  ################################################################################
  # Stabilization terms
  ################################################################################

  # The SUPG stabilization terms for the density equation
  [./rho_supg]
    type = NSSUPGMass
    variable = rho
    rho = rho
    rhou = rhou
    rhov = rhov
    rhoe = rhoe
    u = vel_x
    v = vel_y
    temperature = temperature
    enthalpy = enthalpy
  [../]

  # The SUPG stabilization terms for the x-momentum equation
  [./rhou_supg]
    type = NSSUPGMomentum
    component = 0
    variable = rhou
    rho = rho
    rhou = rhou
    rhov = rhov
    rhoe = rhoe
    u = vel_x
    v = vel_y
    temperature = temperature
    enthalpy = enthalpy
  [../]

  # The SUPG stabilization terms for the y-momentum equation
  [./rhov_supg]
    type = NSSUPGMomentum
    component = 1
    variable = rhov
    rho = rho
    rhou = rhou
    rhov = rhov
    rhoe = rhoe
    u = vel_x
    v = vel_y
    temperature = temperature
    enthalpy = enthalpy
  [../]

  # The SUPG stabilization terms for the energy equation
  [./rhoe_supg]
    type = NSSUPGEnergy
    variable = rhoe
    rho = rho
    rhou = rhou
    rhov = rhov
    rhoe = rhoe
    u = vel_x
    v = vel_y
    temperature = temperature
    enthalpy = enthalpy
  [../]
[]



[AuxKernels]
  [./u_vel]
    type = NSVelocityAux
    variable = vel_x
    rho = rho
    momentum = rhou
  [../]

  [./v_vel]
    type = NSVelocityAux
    variable = vel_y
    rho = rho
    momentum = rhov
  [../]

  [./temperature_auxkernel]
    type = NSTemperatureAux
    variable = temperature
    rho = rho
    u = vel_x
    v = vel_y
    rhoe = rhoe
  [../]

  [./pressure_auxkernel]
    type = NSPressureAux
    variable = pressure
    rho = rho
    u = vel_x
    v = vel_y
    rhoe = rhoe
  [../]

  [./enthalpy_auxkernel]
    type = NSEnthalpyAux
    variable = enthalpy
    rho = rho
    rhoe = rhoe
    pressure = pressure
  [../]

  [./mach_auxkernel]
    type = NSMachAux
    variable = Mach
    u = vel_x
    v = vel_y
    temperature = temperature
  [../]
[]



[BCs]
  # "Free outflow" mass equation BC
  [./mass_outflow]
    type = NSMassUnspecifiedNormalFlowBC
    variable = rho
    rho = rho
    rhou = rhou
    rhov = rhov
    rhoe = rhoe
    u = vel_x
    v = vel_y
    boundary = '2' # 'Outflow'
  [../]

  # Specified pressure x-momentum equation invsicid outflow BC
  [./rhou_specified_pressure_outflow]
    type = NSMomentumInviscidSpecifiedPressureBC
    variable = rhou
    rho = rho
    rhou = rhou
    rhov = rhov
    rhoe = rhoe
    u = vel_x
    v = vel_y
    component = 0
    boundary = '2' # 'Outflow'
    specified_pressure = 101325 # Pa
  [../]

  # Specified pressure y-momentum equation inviscid outflow BC
  [./rhov_specified_pressure_outflow]
    type = NSMomentumInviscidSpecifiedPressureBC
    variable = rhov
    rho = rho
    rhou = rhou
    rhov = rhov
    rhoe = rhoe
    u = vel_x
    v = vel_y
    component = 1
    boundary = '2' # 'Outflow'
    specified_pressure = 101325 # Pa
  [../]

  # Specified pressure energy equation outflow BC
  [./rhoe_specified_pressure_outflow]
    type = NSEnergyInviscidSpecifiedPressureBC
    variable = rhoe
    rho = rho
    rhou = rhou
    rhov = rhov
    rhoe = rhoe
    u = vel_x
    v = vel_y
    temperature = temperature
    boundary = '2' # 'Outflow'
    specified_pressure = 101325 # Pa
  [../]

  # The no penentration BC (u.n=0) applies on all the solid surfaces.
  # This is enforced weakly via the NSPressureNeumannBC.
  [./rhou_no_penetration]
    type = NSPressureNeumannBC
    variable = rhou
    component = 0
    boundary = '3 4' # 'Lower Wall, Upper Wall'
    u = vel_x
    v = vel_y
    rho = rho
    rhou = rhou
    rhov = rhov
    rhoe = rhoe
    pressure = pressure
  [../]

  # The no penentration BC (u.n=0) applies on all the solid surfaces.
  # This is enforced weakly via the NSPressureNeumannBC.
  [./rhov_no_penetration]
    type = NSPressureNeumannBC
    variable = rhov
    component = 1
    boundary = '3 4' # 'Lower Wall, Upper Wall'
    u = vel_x
    v = vel_y
    rho = rho
    rhou = rhou
    rhov = rhov
    rhoe = rhoe
    pressure = pressure
  [../]

  #
  # "Weak" stagnation and specified flow direction boundary conditions
  #
  [./weak_stagnation_mass_inflow]
    type = NSMassWeakStagnationBC
    variable = rho
    boundary = '1' # 'Inflow'
    stagnation_pressure = 120192.995549849 # Pa, Mach=0.5 at 1 atm
    stagnation_temperature = 315 # K, Mach=0.5 at 1 atm
    sx = 1.
    sy = 0.
    rho = rho
    rhoe = rhoe
    rhou = rhou
    rhov = rhov
    u = vel_x
    v = vel_y
  [../]

  [./weak_stagnation_rhou_convective_inflow]
    type = NSMomentumConvectiveWeakStagnationBC
    variable = rhou
    component = 0
    boundary = '1' # 'Inflow'
    stagnation_pressure = 120192.995549849 # Pa, Mach=0.5 at 1 atm
    stagnation_temperature = 315 # K, Mach=0.5 at 1 atm
    sx = 1.
    sy = 0.
    rho = rho
    rhoe = rhoe
    rhou = rhou
    rhov = rhov
    u = vel_x
    v = vel_y
  [../]

  [./weak_stagnation_rhou_pressure_inflow]
    type = NSMomentumPressureWeakStagnationBC
    variable = rhou
    component = 0
    boundary = '1' # 'Inflow'
    stagnation_pressure = 120192.995549849 # Pa, Mach=0.5 at 1 atm
    stagnation_temperature = 315 # K, Mach=0.5 at 1 atm
    sx = 1.
    sy = 0.
    rho = rho
    rhoe = rhoe
    rhou = rhou
    rhov = rhov
    u = vel_x
    v = vel_y
  [../]

  [./weak_stagnation_rhov_convective_inflow]
    type = NSMomentumConvectiveWeakStagnationBC
    variable = rhov
    component = 1
    boundary = '1' # 'Inflow'
    stagnation_pressure = 120192.995549849 # Pa, Mach=0.5 at 1 atm
    stagnation_temperature = 315 # K, Mach=0.5 at 1 atm
    sx = 1.
    sy = 0.
    rho = rho
    rhoe = rhoe
    rhou = rhou
    rhov = rhov
    u = vel_x
    v = vel_y
  [../]

  [./weak_stagnation_rhov_pressure_inflow]
    type = NSMomentumPressureWeakStagnationBC
    variable = rhov
    component = 1
    boundary = '1' # 'Inflow'
    stagnation_pressure = 120192.995549849 # Pa, Mach=0.5 at 1 atm
    stagnation_temperature = 315 # K, Mach=0.5 at 1 atm
    sx = 1.
    sy = 0.
    rho = rho
    rhoe = rhoe
    rhou = rhou
    rhov = rhov
    u = vel_x
    v = vel_y
  [../]

  [./weak_stagnation_energy_inflow]
    type = NSEnergyWeakStagnationBC
    variable = rhoe
    boundary = '1' # 'Inflow'
    stagnation_pressure = 120192.995549849 # Pa, Mach=0.5 at 1 atm
    stagnation_temperature = 315 # K, Mach=0.5 at 1 atm
    sx = 1.
    sy = 0.
    rho = rho
    rhoe = rhoe
    rhou = rhou
    rhov = rhov
    u = vel_x
    v = vel_y
  [../]
[]




[Materials]
  [./fluid]
    type = Air
    block = 0 # 'MeshInterior'
    rho = rho
    rhou = rhou
    rhov = rhov
    rhoe = rhoe
    u = vel_x
    v = vel_y
    temperature = temperature
    enthalpy = enthalpy
    # This value is not used in the Euler equations, but it *is* used
    # by the stabilization parameter computation, which it decrease
    # the amount of artificial viscosity added, so it's best to use a
    # realistic value.
    dynamic_viscosity = 0.0
  [../]
[]



[Postprocessors]
  [./entropy_error]
    type = NSEntropyError
    execute_on = 'initial timestep_end'
    block = 0
    rho_infty = 1.1768292682926829
    p_infty = 101325
    rho = rho
    pressure = pressure
  [../]
[]



[Preconditioning]
  [./SMP_PJFNK]
    type = SMP
    full = true
    solve_type = 'PJFNK'
  [../]
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
  [./Quadrature]
    type = TRAP
    order = FIRST
  [../]
[]



[Outputs]
  interval = 1
  exodus = true
[]
