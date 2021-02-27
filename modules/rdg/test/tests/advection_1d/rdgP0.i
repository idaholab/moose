# This test demonstrates the advection of a tracer in 1D using the RDG module.
# There is no slope limiting.  Changing the SlopeLimiting scheme to minmod, mc,
# or superbee means that a linear reconstruction is performed, and the slope
# limited according to the scheme chosen.  Doing this produces RDG(P0P1) and
# substantially reduces numerical diffusion

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 100
  xmin = 0
  xmax = 1
[]

[Variables]
  [./tracer]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[ICs]
  [./tracer]
    type = FunctionIC
    variable = tracer
    function = 'if(x<0.1,0,if(x>0.3,0,1))'
  [../]
[]

[UserObjects]
  [./lslope]
    type = AEFVSlopeLimitingOneD
    execute_on = 'linear'
    scheme = 'none' #none | minmod | mc | superbee
    u = tracer
  [../]

  [./internal_side_flux]
    type = AEFVUpwindInternalSideFlux
    execute_on = 'linear'
    velocity = 0.1
  [../]

  [./free_outflow_bc]
    type = AEFVFreeOutflowBoundaryFlux
    execute_on = 'linear'
    velocity = 0.1
  [../]
[]

[Kernels]
  [./dot]
    type = TimeDerivative
    variable = tracer
  [../]
[]
[DGKernels]
  [./concentration]
    type = AEFVKernel
    variable = tracer
    component = 'concentration'
    flux = internal_side_flux
    u = tracer
  [../]
[]

[BCs]
  [./concentration]
    type = AEFVBC
    boundary = 'left right'
    variable = tracer
    component = 'concentration'
    flux = free_outflow_bc
    u = tracer
  [../]
[]

[Materials]
  [./aefv]
    type = AEFVMaterial
    slope_limiting = lslope
    u = tracer
  [../]
[]


[VectorPostprocessors]
  [./tracer]
    type = LineValueSampler
    start_point = '0 0 0'
    end_point = '1 0 0'
    num_points = 100
    sort_by = x
    variable = tracer
  [../]
[]

[Executioner]
  type = Transient
  solve_type = PJFNK
  end_time = 6
  dt = 6E-1
  nl_abs_tol = 1E-8
  timestep_tolerance = 1E-3
[]

[Outputs]
  #exodus = true
  csv = true
  execute_on = final
[]
