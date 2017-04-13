#### Source:
#### 4th International Workshop on High-Order CFD Methods
#### Website:
#### <https://how4.cenaero.be/>
#### Test case:
#### BI3 - Inviscid bow shock
#### The detached bow shock upstream of a 2D
#### simple blunt body in inviscid conditions is studied.
#### This case is designed to isolate testing of
#### the shock-capturing properties of schemes.
#### This case is computationally expedient, being steady,
#### two-dimensional, inviscid flow,
#### with well-defined boundary conditions.
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
  boundary_condition_user_object_list = 'slip_bcuo
                                         farfield_bcuo
                                         farfield_bcuo
                                         farfield_bcuo'
  infinity_density = 1.
  infinity_x_velocity = 4.
  infinity_pressure = 0.71428571428571428571
  implicit = false
[]
############################################################
[Mesh]
  file = 2d_bowshock_mesh_lv0.msh
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
    type = CNSFVSlipBCUserObject
    execute_on = 'linear'
  [../]

  [./farfield_bcuo]
    type = CNSFVRiemannInvariantBCUserObject
    execute_on = 'linear'
  [../]

  [./rslope]
    type = CNSFVLeastSquaresSlopeReconstruction
    execute_on = 'linear'
  [../]

  [./lslope]
    type = CNSFVWENOSlopeLimiting
    execute_on = 'linear'
  [../]

  [./riemann]
    type = CNSFVHLLCInternalSideFlux
    execute_on = 'linear'
  [../]

  [./farfield_bc]
    type = CNSFVRiemannInvariantBoundaryFlux
    bc_uo = 'farfield_bcuo'
    execute_on = 'linear'
  [../]

  [./slip_bc]
    type = CNSFVHLLCSlipBoundaryFlux
    bc_uo = 'slip_bcuo'
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
    value = 4.0
  [../]

  [./rhov_ic]
    variable = 'momy'
    type = ConstantIC
    value = 0.
  [../]

  [./rhoe_ic]
    variable = 'rhoe'
    type = ConstantIC
    value = 9.78571428571428571429
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

  ## walls

  [./slip_mass]
    type = CNSFVBC
    boundary = '1'
    variable = rho
    component = 'mass'
    flux = slip_bc
  [../]

  [./slip_momx]
    type = CNSFVBC
    boundary = '1'
    variable = momx
    component = 'x-momentum'
    flux = slip_bc
  [../]

  [./slip_momy]
    type = CNSFVBC
    boundary = '1'
    variable = momy
    component = 'y-momentum'
    flux = slip_bc
  [../]

  [./slip_etot]
    type = CNSFVBC
    boundary = '1'
    variable = rhoe
    component = 'total-energy'
    flux = slip_bc
  [../]

  ## far-field

  [./farfield_mass]
    type = CNSFVBC
    boundary = '2 3 4'
    variable = rho
    component = 'mass'
    flux = farfield_bc
  [../]

  [./farfield_momx]
    type = CNSFVBC
    boundary = '2 3 4'
    variable = momx
    component = 'x-momentum'
    flux = farfield_bc
  [../]

  [./farfield_momy]
    type = CNSFVBC
    boundary = '2 3 4'
    variable = momy
    component = 'y-momentum'
    flux = farfield_bc
  [../]

  [./farfield_etot]
    type = CNSFVBC
    boundary = '2 3 4'
    variable = rhoe
    component = 'total-energy'
    flux = farfield_bc
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
    cfl = 0.2
  [../]

  [./DT]
    type = TimestepSize
    execute_on = 'initial timestep_end'
  [../]

  [./L2error]
    type = CNSFVIdealGasTotalEnthalpyL2Error
    execute_on = 'initial timestep_end'
    block = 0
  [../]
[]
############################################################
[Executioner]
  type = Transient
  [./TimeIntegrator]
    type = ExplicitEuler
  [../]
  solve_type = 'linear'

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

  num_steps = 1 # 1 | 100000 for complete run
[]
############################################################
[Outputs]
  [./Exodus]
    type = Exodus
    execute_on = 'initial timestep_end final'
    file_base = 2d_bowshock_expl_lv0_out
    elemental_as_nodal = true
    interval = 1 # 1 | 100 for complete run
  [../]

  [./CSV]
    type = CSV
    file_base = 2d_bowshock_expl_lv0_res
    interval = 10
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
