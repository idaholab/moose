#### 2D steady-state simulation of
#### subsonic inviscid flow past a smooth bump in a channel
#### at freestream Mach number 0.5
############################################################
[GlobalParams]
  order = CONSTANT
  family = MONOMIAL
  rho  = rho
  rhou = momx
  rhov = momy
  rhoe = rhoe
  fluid_properties = fp
  slope_reconstruction = rslope
  slope_limiting = lslope
  boundary_list = '1 2 3 4'
  boundary_condition_user_object_list = 'inflow_bcuo
                                         outflow_bcuo
                                         slip_bcuo
                                         slip_bcuo'
  infinity_density = 1.
  infinity_x_velocity = 0.5
  infinity_pressure = 0.71428571428571428571
[]
############################################################
[Mesh]
  file = SmoothBump_quad_ref1_Q1.msh
[]
############################################################
[Problem]
  kernel_coverage_check = false
[]
############################################################
[Modules]
  [./FluidProperties]
    [./fp]
      type = IdealGasFluidProperties
      gamma = 1.4
      R = 0.71428571428571428571
    [../]
  [../]
[]
############################################################
[UserObjects]
  [./slip_bcuo]
    execute_on = 'linear'
    type = CNSFVSlipBCUserObject
  [../]

  [./inflow_bcuo]
    execute_on = 'linear'
    type = CNSFVRiemannInvariantBCUserObject
  [../]

  [./outflow_bcuo]
    execute_on = 'linear'
    type = CNSFVRiemannInvariantBCUserObject
  [../]

  [./rslope]
    execute_on = 'linear'
    type = CNSFVLeastSquaresSlopeReconstruction
  [../]

  [./lslope]
    execute_on = 'linear'
    type = CNSFVNoSlopeLimiting
  [../]

  [./riemann]
    execute_on = 'linear'
    type = CNSFVHLLCInternalSideFlux
  [../]

  [./inflow_bc]
    execute_on = 'linear'
    type = CNSFVRiemannInvariantBoundaryFlux
    bc_uo = 'inflow_bcuo'
  [../]

  [./outflow_bc]
    execute_on = 'linear'
    type = CNSFVRiemannInvariantBoundaryFlux
    bc_uo = 'outflow_bcuo'
  [../]

  [./slip_bc]
    execute_on = 'linear'
    type = CNSFVHLLCSlipBoundaryFlux
    bc_uo = 'slip_bcuo'
  [../]
[]
############################################################
[Variables]
  [./rho]
  [../]

  [./momx]
  [../]

  [./momy]
  [../]

  [./rhoe]
  [../]
[]
############################################################
[AuxVariables]
  [./mach]
  [../]

  [./pres]
  [../]
[]
############################################################
[ICs]
  [./rho_ic]
    variable = 'rho'
    type = ConstantIC
    value = 1.
  [../]

  [./rhou_ic]
    variable = 'momx'
    type = ConstantIC
    value = 0.5
  [../]

  [./rhov_ic]
    variable = 'momy'
    type = ConstantIC
    value = 0.
  [../]

  [./rhoe_ic]
    variable = 'rhoe'
    type = ConstantIC
    value = 1.91071428571428571429
  [../]

  [./mach_ic]
    type = CNSFVMachIC
    variable = 'mach'
  [../]

  [./pres_ic]
    type = CNSFVPressureIC
    variable = 'pres'
  [../]
[]
############################################################
[Kernels]
  #### Time derivative of mass
  [./time_rho]
    type = TimeDerivative
    variable = rho
  [../]

  #### Time derivative of momentum in x-direction
  [./time_momx]
    type = TimeDerivative
    variable = momx
  [../]

  #### Time derivative of momentum in y-direction
  [./time_momy]
    type = TimeDerivative
    variable = momy
  [../]

  #### Time derivative of total energy
  [./time_rhoe]
    type = TimeDerivative
    variable = rhoe
  [../]
[]
############################################################
[DGKernels]
  #### Mass conservation eqn
  [./mass]
    type = CNSFVKernel
    variable = rho
    component = 'mass'
    flux = riemann
  [../]

  #### Momentum balance eqn in x-direction
  [./momx]
    type = CNSFVKernel
    variable = momx
    component = 'x-momentum'
    flux = riemann
  [../]

  #### Momentum balance eqn in y-direction
  [./momy]
    type = CNSFVKernel
    variable = momy
    component = 'y-momentum'
    flux = riemann
  [../]

  #### Total energy conservation eqn
  [./etot]
    type = CNSFVKernel
    variable = rhoe
    component = 'total-energy'
    flux = riemann
  [../]
[]
############################################################
[AuxKernels]
  [./mach]
    type = CNSFVMachAux
    variable = mach
  [../]

  [./pres]
    type = CNSFVPressureAux
    variable = pres
  [../]
[]
############################################################
[BCs]

  ## walls

  [./slip_mass]
    type = CNSFVBC
    boundary = '3 4'
    variable = rho
    component = 'mass'
    flux = slip_bc
  [../]

  [./slip_momx]
    type = CNSFVBC
    boundary = '3 4'
    variable = momx
    component = 'x-momentum'
    flux = slip_bc
  [../]

  [./slip_momy]
    type = CNSFVBC
    boundary = '3 4'
    variable = momy
    component = 'y-momentum'
    flux = slip_bc
  [../]

  [./slip_etot]
    type = CNSFVBC
    boundary = '3 4'
    variable = rhoe
    component = 'total-energy'
    flux = slip_bc
  [../]

  ## inflow

  [./inflow_mass]
    type = CNSFVBC
    boundary = '1'
    variable = rho
    component = 'mass'
    flux = inflow_bc
  [../]

  [./inflow_momx]
    type = CNSFVBC
    boundary = '1'
    variable = momx
    component = 'x-momentum'
    flux = inflow_bc
  [../]

  [./inflow_momy]
    type = CNSFVBC
    boundary = '1'
    variable = momy
    component = 'y-momentum'
    flux = inflow_bc
  [../]

  [./inflow_etot]
    type = CNSFVBC
    boundary = '1'
    variable = rhoe
    component = 'total-energy'
    flux = inflow_bc
  [../]

  ## outflow

  [./outflow_mass]
    type = CNSFVBC
    boundary = '2'
    variable = rho
    component = 'mass'
    flux = outflow_bc
  [../]

  [./outflow_momx]
    type = CNSFVBC
    boundary = '2'
    variable = momx
    component = 'x-momentum'
    flux = outflow_bc
  [../]

  [./outflow_momy]
    type = CNSFVBC
    boundary = '2'
    variable = momy
    component = 'y-momentum'
    flux = outflow_bc
  [../]

  [./outflow_etot]
    type = CNSFVBC
    boundary = '2'
    variable = rhoe
    component = 'total-energy'
    flux = outflow_bc
  [../]
[]
############################################################
[Materials]
  [./cnsfv]
    type = CNSFVMaterial
    block = 0
  [../]
[]
############################################################
[Postprocessors]
  [./dt]
    type = CNSFVTimeStepLimit
    execute_on = 'initial timestep_end'
    cfl = 20
  [../]

  [./DT]
    type = TimestepSize
    execute_on = 'initial timestep_end'
  [../]

  [./L2error]
    type = CNSFVIdealGasEntropyL2Error
    execute_on = 'initial timestep_end'
    block = 0
  [../]
[]
############################################################
[Preconditioning]
  [./SMP]
    type = SMP
    full = true
    solve_type = 'NEWTON'
  [../]
[]
############################################################
[Executioner]
  type = Transient
  [./TimeIntegrator]
    type = ImplicitEuler
  [../]

  [./TimeStepper]
    type = PostprocessorDT
    postprocessor = dt
  [../]

  l_tol = 1e-2
  l_max_its = 20
  nl_rel_tol = 1e-2
  nl_abs_tol = 1e-7
  nl_max_its = 50

  trans_ss_check = true
  ss_check_tol = 1e-12

  num_steps = 1 # 1 | 100
[]
############################################################
[Outputs]
  [./Exodus]
    type = Exodus
    execute_on = 'initial timestep_end final'
    file_base = 2d_bump_impl_ref1_out
    elemental_as_nodal = true
    interval = 1
  [../]

  [./CSV]
    type = CSV
    file_base = 2d_bump_impl_ref1_res
    interval = 1
  [../]

  [./CONSOLE]
    type = Console
    output_linear = true
    output_nonlinear = true
    execute_postprocessors_on = 'none'
    interval = 1
  [../]

  print_perf_log = true
[]
############################################################
