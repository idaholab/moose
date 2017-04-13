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
  nx = 500 # 500 | 4000 as reference solution
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
    xy_data = '0.1 2500
               0.9 0.025
               1.0 250'
  [../]
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

  [./rslope]
    type = CNSFVSlopeReconstructionOneD
    execute_on = 'linear'
  [../]

  [./lslope]
    type = CNSFVSlopeLimitingOneD
    execute_on = 'linear'
    scheme = 'minmod' #none | minmod | mc | superbee
  [../]

  [./riemann]
    type = CNSFVHLLCInternalSideFlux
    execute_on = 'linear'
  [../]

  [./bc]
    type = CNSFVHLLCSlipBoundaryFlux
    bc_uo = 'slip_bcuo'
    execute_on = 'linear'
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
  [./mass]
    type = CNSFVBC
    boundary = 'left right'
    variable = rho
    component = 'mass'
    flux = bc
  [../]

  [./momx]
    type = CNSFVBC
    boundary = 'left right'
    variable = rhou
    component = 'x-momentum'
    flux = bc
  [../]

  [./etot]
    type = CNSFVBC
    boundary = 'left right'
    variable = rhoe
    component = 'total-energy'
    flux = bc
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
  #### t = 0.038 sec
  num_steps = 4 # 4 | 3800 for 500 cells | 19000 for 4000 cells
  dt = 1e-5 # 1e-5 for 500 cells | 2e-6 for 4000 cells
  dtmin = 1e-6
[]

[Outputs]
  [./Exodus]
    type = Exodus
    file_base = 1d_woodward_colella_blast_wave_out
    interval = 1 # 1 | 20 for 500 cells | 100 for 4000 cells
  [../]
  print_perf_log = true
[]
