############################################################
[GlobalParams]
  order = CONSTANT
  family = MONOMIAL
  u = u
  slope_reconstruction = rslope
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

[MeshModifiers]
  [./subdomain1]
    type = SubdomainBoundingBox
    bottom_left = '0.5 0 0'
    block_id = 1
    top_right = '1.0 1.0 0'
  [../]
  [./interface]
    type = SideSetsBetweenSubdomains
    depends_on = subdomain1
    master_block = '0'
    paired_block = '1'
    new_boundary = 'master0_interface'
  [../]
  [./interface_again]
    type = SideSetsBetweenSubdomains
    depends_on = subdomain1
    master_block = '1'
    paired_block = '0'
    new_boundary = 'master1_interface'
  [../]
[]
############################################################
[Functions]
  [./ic_u]
    type = PiecewiseConstant
    axis = 0
    direction = right
    xy_data = '0.1 0.5
               0.4 1.0
               0.5 0.5'
  [../]
[]
############################################################
[UserObjects]

  [./rslope]
    type = AEFVSlopeReconstructionOneD
    execute_on = 'linear'
    block = 0
  [../]

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
  [../]
  [./diff_v]
    implicit = true
    type = Diffusion
    variable = v
  [../]
  [./time_v]
    implicit = true
    type = TimeDerivative
    variable = v
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
    boundary = 'left master0_interface'
    variable = u
    component = 'concentration'
    flux = free_outflow_bc
  [../]
  [./v_left]
    type = DirichletBC
    boundary = 'master1_interface'
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
  print_perf_log = true
[]
