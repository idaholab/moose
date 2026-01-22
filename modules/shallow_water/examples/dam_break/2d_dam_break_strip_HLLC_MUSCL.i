# 1D dam-break embedded in a thin 2D strip (MUSCL + HLLC)

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 200
  ny = 5
  xmax = 1.0
  ymax = 0.02
[]

[GlobalParams]
  family = MONOMIAL
  order = CONSTANT
[]

[Variables]
  [h]
  []
  [hu]
  []
  [hv]
  []
[]

[Functions]
  [flat]
    type = ConstantFunction
    value = 0.0
  []
  [hL]
    type = ConstantFunction
    value = 1.0
  []
  [hR]
    type = ConstantFunction
    value = 0.5
  []
  [h_init]
    type = ParsedFunction
    value = "if(x<0.5, hL, hR)"
    vars = 'hL hR'
    vals = 'hL hR'
  []
[]

[UserObjects]
  [flux]
    type = SWENumericalFluxHLLC
    use_pvrs = true
    degeneracy_eps = 1e-10
    blend_alpha = 0.0
    log_debug = false
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [recon2d]
    type = SlopeReconstructionMultiDSWE
    h = h
    hu = hu
    hv = hv
    min_neighbors = 2
    weight_model = inverse_distance2
    dry_depth = 1e-6
    positivity_guard = true
    boundary_list = ''
    boundary_condition_user_object_list = ''
    execute_on = 'INITIAL TIMESTEP_BEGIN TIMESTEP_END'
  []
  [limiter2d]
    type = SlopeLimitingBarthJespersen
    slope_reconstruction = recon2d
    execute_on = 'INITIAL TIMESTEP_BEGIN TIMESTEP_END'
  []
  [wall]
    type = SWEWallBoundaryFlux
  []
[]

[ICs]
  [h0]
    type = FunctionIC
    variable = h
    function = h_init
  []
  [hu0]
    type = ConstantIC
    variable = hu
    value = 0.0
  []
  [hv0]
    type = ConstantIC
    variable = hv
    value = 0.0
  []
[]

[Materials]
  [recon]
    type = SWERDGReconstruction
    h = h
    hu = hu
    hv = hv
    slope_limiting = limiter2d
  []
[]

[AuxVariables]
  [b_field]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [b_out]
    type = FunctionAux
    variable = b_field
    function = flat
  []
[]

[DGKernels]
  [flux_h]
    type = SWEFVFluxDGKernel
    variable = h
    h = h
    hu = hu
    hv = hv
    b_var = b_field
    numerical_flux = flux
  []
  [flux_hu]
    type = SWEFVFluxDGKernel
    variable = hu
    h = h
    hu = hu
    hv = hv
    b_var = b_field
    numerical_flux = flux
  []
  [flux_hv]
    type = SWEFVFluxDGKernel
    variable = hv
    h = h
    hu = hu
    hv = hv
    b_var = b_field
    numerical_flux = flux
  []
[]

[BCs]
  active = 'bch bchu bchv'
  [bch]
    type = SWEFluxBC
    variable = h
    boundary = 'left right top bottom'
    h = h
    hu = hu
    hv = hv
    boundary_flux = wall
  []
  [bchu]
    type = SWEFluxBC
    variable = hu
    boundary = 'left right top bottom'
    h = h
    hu = hu
    hv = hv
    boundary_flux = wall
  []
  [bchv]
    type = SWEFluxBC
    variable = hv
    boundary = 'left right top bottom'
    h = h
    hu = hu
    hv = hv
    boundary_flux = wall
  []
[]

[Kernels]
  [th]
    type = TimeDerivative
    variable = h
  []
  [thu]
    type = TimeDerivative
    variable = hu
  []
  [thv]
    type = TimeDerivative
    variable = hv
  []
[]

[VectorPostprocessors]
  [h_line]
    type = LineValueSampler
    start_point = '${fparse 0.5 * ${Mesh/xmax} / ${Mesh/nx}} 0.01 0.0'
    end_point   = '${fparse ${Mesh/xmax} * (1 - 0.5 / ${Mesh/nx})} 0.01 0.0'
    num_points = ${Mesh/nx}
    sort_by = x
    variable = h
  []
[]

[Executioner]
  type = Transient
  dt = 1e-3
  nl_abs_tol = 1e-12
  num_steps = 100
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
[]

[Outputs]
  csv = true
  exodus = true
  print_linear_residuals = false
[]

