#### 2D transient simulation of
#### Mach 3 wind tunnel with a forward step
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
  boundary_condition_user_object_list = 'wall_bcuo
                                         wall_bcuo
                                         inflow_bcuo
                                         outflow_bcuo'
  infinity_density = 1.
  infinity_x_velocity = 3.
  infinity_pressure = 0.71428571428571428571
  implicit = false
[]
############################################################
[Mesh]
  file = 2d_mach3step.e
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
  [./wall_bcuo]
    type = CNSFVSlipBCUserObject
    execute_on = 'linear'
  [../]

  [./inflow_bcuo]
    type = CNSFVRiemannInvariantBCUserObject
    execute_on = 'linear'
  [../]

  [./outflow_bcuo]
    type = CNSFVRiemannInvariantBCUserObject
    execute_on = 'linear'
  [../]

  [./rslope]
    type = CNSFVLeastSquaresSlopeReconstruction
    execute_on = 'linear'
  [../]

  [./lslope]
    type = CNSFVMinmaxSlopeLimiting
    execute_on = 'linear'
  [../]

  [./riemann]
    type = CNSFVHLLCInternalSideFlux
    execute_on = 'linear'
  [../]

  [./wall_bc]
    type = CNSFVHLLCSlipBoundaryFlux
    bc_uo = 'wall_bcuo'
    execute_on = 'linear'
  [../]

  [./inflow_bc]
    type = CNSFVRiemannInvariantBoundaryFlux
    bc_uo = 'inflow_bcuo'
    execute_on = 'linear'
  [../]

  [./outflow_bc]
    type = CNSFVRiemannInvariantBoundaryFlux
    bc_uo = 'outflow_bcuo'
    execute_on = 'linear'
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
    value = 3.
  [../]

  [./rhov_ic]
    variable = 'momy'
    type = ConstantIC
    value = 0.
  [../]

  [./rhoe_ic]
    variable = 'rhoe'
    type = ConstantIC
    value = 6.28571428571428571429
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
    implicit = true
    type = TimeDerivative
    variable = rho
  [../]

  #### Time derivative of momentum in x-direction
  [./time_momx]
    implicit = true
    type = TimeDerivative
    variable = momx
  [../]

  #### Time derivative of momentum in y-direction
  [./time_momy]
    implicit = true
    type = TimeDerivative
    variable = momy
  [../]

  #### Time derivative of total energy
  [./time_rhoe]
    implicit = true
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

  ## wall

  [./wall_mass]
    type = CNSFVBC
    boundary = '1 2'
    variable = rho
    component = 'mass'
    flux = wall_bc
  [../]

  [./wall_momx]
    type = CNSFVBC
    boundary = '1 2'
    variable = momx
    component = 'x-momentum'
    flux = wall_bc
  [../]

  [./wall_momy]
    type = CNSFVBC
    boundary = '1 2'
    variable = momy
    component = 'y-momentum'
    flux = wall_bc
  [../]

  [./wall_etot]
    type = CNSFVBC
    boundary = '1 2'
    variable = rhoe
    component = 'total-energy'
    flux = wall_bc
  [../]

  ## inflow

  [./inflow_mass]
    type = CNSFVBC
    boundary = '3'
    variable = rho
    component = 'mass'
    flux = inflow_bc
  [../]

  [./inflow_momx]
    type = CNSFVBC
    boundary = '3'
    variable = momx
    component = 'x-momentum'
    flux = inflow_bc
  [../]

  [./inflow_momy]
    type = CNSFVBC
    boundary = '3'
    variable = momy
    component = 'y-momentum'
    flux = inflow_bc
  [../]

  [./inflow_etot]
    type = CNSFVBC
    boundary = '3'
    variable = rhoe
    component = 'total-energy'
    flux = inflow_bc
  [../]

  ## outflow

  [./outflow_mass]
    type = CNSFVBC
    boundary = '4'
    variable = rho
    component = 'mass'
    flux = outflow_bc
  [../]

  [./outflow_momx]
    type = CNSFVBC
    boundary = '4'
    variable = momx
    component = 'x-momentum'
    flux = outflow_bc
  [../]

  [./outflow_momy]
    type = CNSFVBC
    boundary = '4'
    variable = momy
    component = 'y-momentum'
    flux = outflow_bc
  [../]

  [./outflow_etot]
    type = CNSFVBC
    boundary = '4'
    variable = rhoe
    component = 'total-energy'
    flux = outflow_bc
  [../]
[]
############################################################
[Materials]
  [./cnsfv]
    type = CNSFVMaterial
    block = 1
  [../]
[]
############################################################
[Postprocessors]
  [./dt]
    type = CNSFVTimeStepLimit
    execute_on = 'initial timestep_end'
    cfl = 0.5
  [../]
[]
############################################################
[Executioner]
  type = Transient
  solve_type = 'LINEAR'

  [./TimeIntegrator]
    type = ExplicitTVDRK2
  [../]

  [./TimeStepper]
    type = PostprocessorDT
    postprocessor = dt
  [../]

  l_tol = 1e-4
  trans_ss_check = true
  ss_check_tol = 1e-12

  start_time = 0.
  end_time = 5.
  num_steps = 1 # 1 | 20000
[]
############################################################
[Outputs]
  [./Exodus]
    type = Exodus
    execute_on = 'initial timestep_end final'
    file_base = 2d_mach3step_out
    elemental_as_nodal = true
    interval = 1 # 1 | 20
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
