############################################################
[GlobalParams]
  order = CONSTANT
  family = MONOMIAL
  u = u
  slope_limiting = lslope
  implicit = false
[]
############################################################
[Mesh]
  type = GeneratedMesh
  dim = 1
  xmin = 0
  xmax = 1
  nx = 100
[]
############################################################
[Functions]
  [./ic_u]
    type = PiecewiseConstant
    axis = x
    direction = right
    xy_data = '0.1 0.5
               0.6 1.0
               1.0 0.5'
  [../]
[]
############################################################
[UserObjects]
  [./lslope]
    type = AEFVSlopeLimitingOneD
    execute_on = 'linear'
    scheme = 'none' #none | minmod | mc | superbee
  [../]

  [./internal_side_flux]
    type = AEFVUpwindInternalSideFlux
    execute_on = 'linear'
  [../]

  [./free_outflow_bc]
    type = AEFVFreeOutflowBoundaryFlux
    execute_on = 'linear'
  [../]
[]
############################################################
[Variables]
  [./u]
  [../]
[]
############################################################
[ICs]
  [./u_ic]
    type = FunctionIC
    variable = 'u'
    function = ic_u
  [../]
[]
############################################################
[Kernels]
  [./time_u]
    implicit = true
    type = TimeDerivative
    variable = u
  [../]
[]
############################################################
[DGKernels]
  [./concentration]
    type = AEFVKernel
    variable = u
    component = 'concentration'
    flux = internal_side_flux
  [../]
[]
############################################################
[BCs]
  [./concentration]
    type = AEFVBC
    boundary = 'left right'
    variable = u
    component = 'concentration'
    flux = free_outflow_bc
  [../]
[]
############################################################
[Materials]
  [./aefv]
    type = AEFVMaterial
    block = 0
  [../]
[]
############################################################
[Executioner]
  type = Transient
  [./TimeIntegrator]
    type = ExplicitMidpoint
  [../]
  solve_type = 'LINEAR'

  l_tol = 1e-4
  nl_rel_tol = 1e-20
  nl_abs_tol = 1e-8
  nl_max_its = 60

  start_time = 0.0
  num_steps = 4 # 4 | 400 for complete run
  dt = 5e-4
  dtmin = 1e-6
[]

[Outputs]
  [./Exodus]
    type = Exodus
    file_base = 1d_aefv_square_wave_none_out
    interval = 2
  [../]
  perf_graph = true
[]
