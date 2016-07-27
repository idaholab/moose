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


[NavierStokes]
  [./Variables]
    #         'rho rhou rhov   rhoE'
    scaling = '1.  1.    1.    9.869232667160121e-6'
    family = LAGRANGE
    order = FIRST
  [../]
  [./ICs]
    initial_pressure = 101325.
    initial_temperature = 300.
    initial_velocity = '173.594354746921 0 0' # Mach 0.5: = 0.5*sqrt(gamma*R*T)
    fluid_properties = ideal_gas
  [../]
  [./Kernels]
    fluid_properties = ideal_gas
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
