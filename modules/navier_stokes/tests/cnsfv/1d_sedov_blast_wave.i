#### The initial total energy is adjustable.
#### It can range from small (10) to large (1,000,000).
#### But a larger initial total energy would require
#### a smaller initial time step size.
#### An initial condition with rhoe = 1,000,000 requires
#### a time step size much smaller than that with rhoe = 10.
#### This test case present a scenario with
#### a small initial total energy for quick validation.

############################################################
[GlobalParams]
  order = CONSTANT
  family = MONOMIAL
  rho  = rho
  rhou = rhou
  rhoe = rhoe
  slope_reconstruction = rslope
  slope_limiting = lslope
  fluid_properties = fp
  implicit = false
[]
############################################################
[Mesh]
  type = GeneratedMesh
  dim = 1
  xmin = 0
  xmax = 1
  nx = 400 # 400 | 800 as ref soln
[]
############################################################
[Problem]
  kernel_coverage_check = false
[]
############################################################
[Functions]
  [./ic_rho]
    type = PiecewiseConstant
    axis = 0
    direction = right
    xy_data = '1.0 1.0'
  [../]

  [./ic_rhou]
    type = PiecewiseConstant
    axis = 0
    direction = right
    xy_data = '1.0 0.0'
  [../]

  [./ic_rhoe]
    type = PiecewiseConstant
    axis = 0
    direction = right
    xy_data = '0.0025 2387324
               1.2 0.00000001'
  [../]
[]
############################################################
[Modules]
  [./FluidProperties]
    [./fp]
      type = IdealGasFluidProperties
      gamma = 1.66666666666666666667
      R = 0.71428571428571428571
    [../]
  [../]
[]
############################################################
[UserObjects]

  [./symm_bcuo]
    execute_on = 'linear'
    type = CNSFVSlipBCUserObject
  [../]

  [./rslope]
    execute_on = 'linear'
    type = CNSFVSlopeReconstructionOneD
  [../]

  [./lslope]
    execute_on = 'linear'
    type = CNSFVSlopeLimitingOneD
    scheme = 'minmod' #none | minmod | mc | superbee
  [../]

  [./riemann]
    execute_on = 'linear'
    type = CNSFVHLLCInternalSideFlux
  [../]

  [./free_bc]
    execute_on = 'linear'
    type = CNSFVFreeOutflowBoundaryFlux
  [../]

  [./symm_bc]
    execute_on = 'linear'
    type = CNSFVHLLCSlipBoundaryFlux
    bc_uo = 'symm_bcuo'
  [../]
[]
############################################################
[Variables]
  [./rho]
  [../]

  [./rhou]
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
    type = FunctionIC
    variable = 'rho'
    function = ic_rho
  [../]

  [./rhou_ic]
    type = FunctionIC
    variable = 'rhou'
    function = ic_rhou
  [../]

  [./rhoe_ic]
    type = FunctionIC
    variable = 'rhoe'
    function = ic_rhoe
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
  [./time_rhou]
    implicit = true
    type = TimeDerivative
    variable = rhou
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
    variable = rhou
    component = 'x-momentum'
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
  [./left_mass]
    type = CNSFVBC
    boundary = 'left'
    variable = rho
    component = 'mass'
    flux = symm_bc
  [../]

  [./left_momx]
    type = CNSFVBC
    boundary = 'left'
    variable = rhou
    component = 'x-momentum'
    flux = symm_bc
  [../]

  [./left_etot]
    type = CNSFVBC
    boundary = 'left'
    variable = rhoe
    component = 'total-energy'
    flux = symm_bc
  [../]

  [./right_mass]
    type = CNSFVBC
    boundary = 'right'
    variable = rho
    component = 'mass'
    flux = free_bc
  [../]

  [./right_momx]
    type = CNSFVBC
    boundary = 'right'
    variable = rhou
    component = 'x-momentum'
    flux = free_bc
  [../]

  [./right_etot]
    type = CNSFVBC
    boundary = 'right'
    variable = rhoe
    component = 'total-energy'
    flux = free_bc
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
[Executioner]
  type = Transient
  [./TimeIntegrator]
    type = ExplicitTVDRK2
  [../]
  solve_type = 'LINEAR'

  l_tol = 1e-4
  nl_rel_tol = 1e-20
  nl_abs_tol = 1e-8
  nl_max_its = 60

  start_time = 0.0
  ### termination time = 0.005 sec
  num_steps = 4  # 4 | 5000 for 400 cells | 10000 for 800 cells
  dt = 1e-6      # 1e-6 for 500 cells | 5e-7 for 800 cells
  dtmin = 5e-7
[]

[Outputs]
  [./Exodus]
    type = Exodus
    file_base = 1d_sedov_blast_wave_out
    interval = 1 # 1 | 50 for 400 cells | 100 for 800 cells
  [../]
  print_perf_log = true
[]
