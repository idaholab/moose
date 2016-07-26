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
    # [./InitialCondition]
    #   type = ConstantIC
    #   value = 204.290917476559 # Mach 0.5: rho * (c/2) = rho * 0.5 * sqrt(gamma*R*T) = 204.73
    # [../]
  [../]

  [./rhov]
    order = FIRST
    family = LAGRANGE
  [../]

  [./rhoE]
    order = FIRST
    family = LAGRANGE

    # The magnitude of this equation is much larger than the others,
    # so we can scale it by the atmospheric value to get things closer
    # to the same size in the residual vector.
    scaling = ${/ 1. 101325.}

    # p=1 atm everywhere
    [./InitialCondition]
      type = ConstantIC
      # Choose the first value for zero initial flow
      value = 253312.5 # (m^2/s^2) = P / (gamma-1) = rho * c_v * T, with zero flow, T=300K
      # value = 271044.375 # rho * (c_v*T + 0.5*(0.5*gamma*R*T)), with initial flow everywhere
    [../]
  [../]
[]



[AuxVariables]
  [./vel_x]
    order = FIRST
    family = LAGRANGE

    # Comment out this section to start with zero initial flow
    # [./InitialCondition]
    #   type = ConstantIC
    #   value = 173.594354746921 # Mach 0.5: = 0.5*sqrt(gamma*R*T)
    # [../]
  [../]

  [./vel_y]
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
    #   = ( e + 1/2*|u|^2    ) + p/rho
    #   = ( c_v*T + 1/2*|u|^2) + p/rho
    [./InitialCondition]
      type = ConstantIC
      # Choose the first value for zero initial flow
      value = 301350.0 # (m^2/s^2), with zero initial velocity = (cv + R)*T
      # value = 316417.5 # = (cv+R)*T + 0.5 * 0.5 * (gamma*R*T), with initial M=0.5 flow everywhere
    [../]
  [../]

  [./Mach]
    order = FIRST
    family = LAGRANGE
  [../]

  [./internal_energy]
    [./InitialCondition]
      type = ConstantIC
      value = 230317.5 # M=0 everywhere, echo "271044.375/1.17682926829268" | bc -l
      # value = 215250. # M=0.5 everywhere, echo "271044.375/1.17682926829268 - 0.5*173.594354746921*173.594354746921" | bc -l
    [../]
  [../]

  [./specific_volume]
    [./InitialCondition]
      type = ConstantIC
      value = 0.84974093264248915997 # m^3/kg, echo "1/1.17682926829268" | bc -l
    [../]
  [../]
[]



[Modules]
  [./FluidProperties]
    [./ideal_gas]
      type = IdealGasFluidProperties
      gamma = 1.4
      R = 287
    [../]
  [../]
[]


[Kernels]
  ################################################################################
  # Mass conservation Equation
  ################################################################################

  # Time derivative term
  [./rho_time_deriv]
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
    rhoE = rhoE
    u = vel_x
    v = vel_y
    fluid_properties = ideal_gas
  [../]

  ################################################################################
  # x-momentum equation
  ################################################################################

  # Time derivative term
  [./rhou_time_deriv]
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
    rhoE = rhoE
    pressure = pressure
    component = 0
    fluid_properties = ideal_gas
  [../]

  ################################################################################
  # y-momentum equation
  ################################################################################

  # Time derivative term
  [./rhov_time_deriv]
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
    rhoE = rhoE
    pressure = pressure
    component = 1
    fluid_properties = ideal_gas
  [../]


  ################################################################################
  # Total Energy Equation
  ################################################################################

  # Time derivative term
  [./rhoE_time_deriv]
    type = TimeDerivative
    variable = rhoE
  [../]

  # Energy equation inviscid flux term (integrated by parts)
  [./rhoE_if]
    type = NSEnergyInviscidFlux
    variable = rhoE
    rho = rho
    rhou = rhou
    rhov = rhov
    rhoE = rhoE
    u = vel_x
    v = vel_y
    enthalpy = enthalpy
    fluid_properties = ideal_gas
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
    rhoE = rhoE
    u = vel_x
    v = vel_y
    temperature = temperature
    enthalpy = enthalpy
    fluid_properties = ideal_gas
  [../]

  # The SUPG stabilization terms for the x-momentum equation
  [./rhou_supg]
    type = NSSUPGMomentum
    component = 0
    variable = rhou
    rho = rho
    rhou = rhou
    rhov = rhov
    rhoE = rhoE
    u = vel_x
    v = vel_y
    temperature = temperature
    enthalpy = enthalpy
    fluid_properties = ideal_gas
  [../]

  # The SUPG stabilization terms for the y-momentum equation
  [./rhov_supg]
    type = NSSUPGMomentum
    component = 1
    variable = rhov
    rho = rho
    rhou = rhou
    rhov = rhov
    rhoE = rhoE
    u = vel_x
    v = vel_y
    temperature = temperature
    enthalpy = enthalpy
    fluid_properties = ideal_gas
  [../]

  # The SUPG stabilization terms for the energy equation
  [./rhoE_supg]
    type = NSSUPGEnergy
    variable = rhoE
    rho = rho
    rhou = rhou
    rhov = rhov
    rhoE = rhoE
    u = vel_x
    v = vel_y
    temperature = temperature
    enthalpy = enthalpy
    fluid_properties = ideal_gas
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
    internal_energy = internal_energy
    specific_volume = specific_volume
    fluid_properties = ideal_gas
  [../]

  [./pressure_auxkernel]
    type = NSPressureAux
    variable = pressure
    internal_energy = internal_energy
    specific_volume = specific_volume
    fluid_properties = ideal_gas
  [../]

  [./enthalpy_auxkernel]
    type = NSEnthalpyAux
    variable = enthalpy
    rho = rho
    rhoE = rhoE
    pressure = pressure
  [../]

  [./mach_auxkernel]
    type = NSMachAux
    variable = Mach
    u = vel_x
    v = vel_y
    internal_energy = internal_energy
    specific_volume = specific_volume
    fluid_properties = ideal_gas
  [../]

  [./internal_energy_auxkernel]
    type = NSInternalEnergyAux
    variable = internal_energy
    rho = rho
    u = vel_x
    v = vel_y
    rhoE = rhoE
  [../]

  [./specific_volume_auxkernel]
    type = NSSpecificVolumeAux
    variable = specific_volume
    rho = rho
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
    rhoE = rhoE
    u = vel_x
    v = vel_y
    boundary = 'right'
    fluid_properties = ideal_gas
  [../]

  # Specified pressure x-momentum equation invsicid outflow BC
  [./rhou_specified_pressure_outflow]
    type = NSMomentumInviscidSpecifiedPressureBC
    variable = rhou
    rho = rho
    rhou = rhou
    rhov = rhov
    rhoE = rhoE
    u = vel_x
    v = vel_y
    component = 0
    boundary = 'right'
    specified_pressure = 101325 # Pa
    fluid_properties = ideal_gas
  [../]

  # Specified pressure y-momentum equation inviscid outflow BC
  [./rhov_specified_pressure_outflow]
    type = NSMomentumInviscidSpecifiedPressureBC
    variable = rhov
    rho = rho
    rhou = rhou
    rhov = rhov
    rhoE = rhoE
    u = vel_x
    v = vel_y
    component = 1
    boundary = 'right'
    specified_pressure = 101325 # Pa
    fluid_properties = ideal_gas
  [../]

  # Specified pressure energy equation outflow BC
  [./rhoE_specified_pressure_outflow]
    type = NSEnergyInviscidSpecifiedPressureBC
    variable = rhoE
    rho = rho
    rhou = rhou
    rhov = rhov
    rhoE = rhoE
    u = vel_x
    v = vel_y
    temperature = temperature
    boundary = 'right'
    specified_pressure = 101325 # Pa
    fluid_properties = ideal_gas
  [../]

  # The no penentration BC (u.n=0) applies on all the solid surfaces.
  # This is enforced weakly via the NSPressureNeumannBC.
  [./rhou_no_penetration]
    type = NSPressureNeumannBC
    variable = rhou
    component = 0
    boundary = 'top bottom step_top step_left step_right'
    u = vel_x
    v = vel_y
    rho = rho
    rhou = rhou
    rhov = rhov
    rhoE = rhoE
    pressure = pressure
    fluid_properties = ideal_gas
  [../]

  # The no penentration BC (u.n=0) applies on all the solid surfaces.
  # This is enforced weakly via the NSPressureNeumannBC.
  [./rhov_no_penetration]
    type = NSPressureNeumannBC
    variable = rhov
    component = 1
    boundary = 'top bottom step_top step_left step_right'
    u = vel_x
    v = vel_y
    rho = rho
    rhou = rhou
    rhov = rhov
    rhoE = rhoE
    pressure = pressure
    fluid_properties = ideal_gas
  [../]

  #
  # "Weak" stagnation and specified flow direction boundary conditions
  #
  [./weak_stagnation_mass_inflow]
    type = NSMassWeakStagnationBC
    variable = rho
    boundary = 'left'
    stagnation_pressure = 120192.995549849 # Pa, Mach=0.5 at 1 atm
    stagnation_temperature = 315 # K, Mach=0.5 at 1 atm
    sx = 1.
    sy = 0.
    rho = rho
    rhoE = rhoE
    rhou = rhou
    rhov = rhov
    u = vel_x
    v = vel_y
    fluid_properties = ideal_gas
  [../]

  [./weak_stagnation_rhou_convective_inflow]
    type = NSMomentumConvectiveWeakStagnationBC
    variable = rhou
    component = 0
    boundary = 'left'
    stagnation_pressure = 120192.995549849 # Pa, Mach=0.5 at 1 atm
    stagnation_temperature = 315 # K, Mach=0.5 at 1 atm
    sx = 1.
    sy = 0.
    rho = rho
    rhoE = rhoE
    rhou = rhou
    rhov = rhov
    u = vel_x
    v = vel_y
    fluid_properties = ideal_gas
  [../]

  [./weak_stagnation_rhou_pressure_inflow]
    type = NSMomentumPressureWeakStagnationBC
    variable = rhou
    component = 0
    boundary = 'left'
    stagnation_pressure = 120192.995549849 # Pa, Mach=0.5 at 1 atm
    stagnation_temperature = 315 # K, Mach=0.5 at 1 atm
    sx = 1.
    sy = 0.
    rho = rho
    rhoE = rhoE
    rhou = rhou
    rhov = rhov
    u = vel_x
    v = vel_y
    fluid_properties = ideal_gas
  [../]

  [./weak_stagnation_rhov_convective_inflow]
    type = NSMomentumConvectiveWeakStagnationBC
    variable = rhov
    component = 1
    boundary = 'left'
    stagnation_pressure = 120192.995549849 # Pa, Mach=0.5 at 1 atm
    stagnation_temperature = 315 # K, Mach=0.5 at 1 atm
    sx = 1.
    sy = 0.
    rho = rho
    rhoE = rhoE
    rhou = rhou
    rhov = rhov
    u = vel_x
    v = vel_y
    fluid_properties = ideal_gas
  [../]

  [./weak_stagnation_rhov_pressure_inflow]
    type = NSMomentumPressureWeakStagnationBC
    variable = rhov
    component = 1
    boundary = 'left'
    stagnation_pressure = 120192.995549849 # Pa, Mach=0.5 at 1 atm
    stagnation_temperature = 315 # K, Mach=0.5 at 1 atm
    sx = 1.
    sy = 0.
    rho = rho
    rhoE = rhoE
    rhou = rhou
    rhov = rhov
    u = vel_x
    v = vel_y
    fluid_properties = ideal_gas
  [../]

  [./weak_stagnation_energy_inflow]
    type = NSEnergyWeakStagnationBC
    variable = rhoE
    boundary = 'left'
    stagnation_pressure = 120192.995549849 # Pa, Mach=0.5 at 1 atm
    stagnation_temperature = 315 # K, Mach=0.5 at 1 atm
    sx = 1.
    sy = 0.
    rho = rho
    rhoE = rhoE
    rhou = rhou
    rhov = rhov
    u = vel_x
    v = vel_y
    fluid_properties = ideal_gas
  [../]
[]




[Materials]
  [./fluid]
    type = Air
    block = 1
    rho = rho
    rhou = rhou
    rhov = rhov
    rhoE = rhoE
    u = vel_x
    v = vel_y
    temperature = temperature
    enthalpy = enthalpy
    # This value is not used in the Euler equations, but it *is* used
    # by the stabilization parameter computation, which it decrease
    # the amount of artificial viscosity added, so it's best to use a
    # realistic value.
    dynamic_viscosity = 0.0
    fluid_properties = ideal_gas
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
  num_steps = 10000
  nl_rel_tol = 1e-5
  nl_abs_tol = 1e-9
  # nl_abs_step_tol = 1e-15
  nl_max_its = 5
  l_tol = 1e-4 # Relative linear tolerance for each Krylov solve
  l_max_its = 100 # Number of linear iterations for each Krylov solve

  # Specify the order as FIRST, otherwise you will get warnings in DEBUG mode...
  [./Quadrature]
    type = TRAP
    order = FIRST
  [../]
[]



[Outputs]
  file_base = step_out
  interval = 1
  exodus = true
[]
