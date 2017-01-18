############################################################
[GlobalParams]
  order = CONSTANT
  family = MONOMIAL
  rho = rho
  rhou = rhou
  rhov = rhov
  rhoe = rhoe
  fluid_properties = fp
  slope_reconstruction = rslope
  slope_limiting = lslope
  boundary_list = 'left
                   right
                   bottom
                   top'
  boundary_condition_user_object_list = 'free_outflow_bcuo
                                         free_outflow_bcuo
                                         free_outflow_bcuo
                                         free_outflow_bcuo'
  implicit = false
[]
############################################################
[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = 0
  xmax = 1
  ymin = 0
  ymax = 1
  nx = 400
  ny = 1
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
    xy_data = '0.5 1.0
               1.0 0.125'
  [../]

  [./ic_rhoe]
    type = PiecewiseConstant
    axis = 0
    direction = right
    xy_data = '0.5 2.5
               1.0 0.25'
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
  [./free_outflow_bcuo]
    execute_on = 'linear'
    type = CNSFVFreeOutflowBCUserObject
  [../]

  [./rslope]
    execute_on = 'linear'
    type = CNSFVGreenGaussSlopeReconstruction
  [../]

  [./lslope]
    execute_on = 'linear'
    type = CNSFVMinmaxSlopeLimiting
  [../]

  [./riemann]
    execute_on = 'linear'
    type = CNSFVHLLCInternalSideFlux
  [../]

  [./free_outflow_bc]
    execute_on = 'linear'
    type = CNSFVFreeOutflowBoundaryFlux
  [../]
[]
############################################################
[Variables]
  [./rho]
  [../]

  [./rhou]
  [../]

  [./rhov]
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
    type = ConstantIC
    variable = 'rhou'
    value = 0.
  [../]

  [./rhov_ic]
    type = ConstantIC
    variable = 'rhov'
    value = 0.
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

  #### Time derivative of momentum in y-direction
  [./time_rhov]
    implicit = true
    type = TimeDerivative
    variable = rhov
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

  #### Momentum balance eqn in y-direction
  [./momy]
    type = CNSFVKernel
    variable = rhov
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
  [./mass]
    type = CNSFVBC
    boundary = 'left right'
    variable = rho
    component = 'mass'
    flux = free_outflow_bc
  [../]

  [./momx]
    type = CNSFVBC
    boundary = 'left right'
    variable = rhou
    component = 'x-momentum'
    flux = free_outflow_bc
  [../]

  [./momy]
    type = CNSFVBC
    boundary = 'left right'
    variable = rhov
    component = 'y-momentum'
    flux = free_outflow_bc
  [../]

  [./etot]
    type = CNSFVBC
    boundary = 'left right'
    variable = rhoe
    component = 'total-energy'
    flux = free_outflow_bc
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
  #### final time = 0.2
  num_steps = 4 # 4 | 1000 for 400 cells
  dt = 2e-4 # 2e-4 for 400 cells
  dtmin = 1e-6
[]

[Outputs]
  [./Exodus]
    type = Exodus
    file_base = 2d_sod_shock_tube_out
    interval = 1 # 1 | 10 for 400 cells
  [../]
  print_perf_log = true
[]
