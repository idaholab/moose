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
  [gen]
    type = GeneratedMeshGenerator
    dim = 1
    xmin = 0
    xmax = 1
    nx = 100
  []
  [./subdomain1]
    type = SubdomainBoundingBoxGenerator
    bottom_left = '0.5 0 0'
    block_id = 1
    top_right = '1.0 1.0 0'
    input = gen
  [../]
  [./interface]
    type = SideSetsBetweenSubdomainsGenerator
    primary_block = '0'
    paired_block = '1'
    new_boundary = 'primary0_interface'
    input = subdomain1
  [../]
  [./interface_again]
    type = SideSetsBetweenSubdomainsGenerator
    primary_block = '1'
    paired_block = '0'
    new_boundary = 'primary1_interface'
    input = interface
  [../]
[]
############################################################
[Functions]
  [./ic_u]
    type = PiecewiseConstant
    axis = x
    direction = right
    xy_data = '0.1 0.5
               0.4 1.0
               0.5 0.5'
  [../]
[]
############################################################
[UserObjects]
  [./lslope]
    type = AEFVSlopeLimitingOneD
    execute_on = 'linear'
    scheme = 'superbee' #none | minmod | mc | superbee
    block = 0
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
    block = 0
  [../]
  [./v]
    block = 1
    family = LAGRANGE
    order = FIRST
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
    block = 0
  [../]
  [./diff_v]
    implicit = true
    type = Diffusion
    variable = v
    block = 1
  [../]
  [./time_v]
    implicit = true
    type = TimeDerivative
    variable = v
    block = 1
  [../]
[]
############################################################
[DGKernels]
  [./concentration]
    type = AEFVKernel
    variable = u
    component = 'concentration'
    flux = internal_side_flux
    block = 0
  [../]
[]
############################################################
[BCs]
  [./concentration]
    type = AEFVBC
    boundary = 'left primary0_interface'
    variable = u
    component = 'concentration'
    flux = free_outflow_bc
  [../]
  [./v_left]
    type = DirichletBC
    boundary = 'primary1_interface'
    variable = v
    value = 1
  [../]
  [./v_right]
    type = DirichletBC
    boundary = 'right'
    variable = v
    value = 0
  [../]
[]
############################################################
[Materials]
  [./aefv]
    type = AEFVMaterial
    block = 0
  [../]
  [./dummy_1]
    type = GenericConstantMaterial
    block = 1
    prop_names = ''
    prop_values = ''
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
  [./out]
    type = Exodus
    interval = 2
  [../]
  perf_graph = true
[]
