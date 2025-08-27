[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 50
  ny = 3
  xmax = 1.0
  ymax = 0.1
[]

[GlobalParams]
  family = MONOMIAL
  order = CONSTANT
[]

[Variables]
  [h]   # Water depth (m)
  []
  [hu]  # Depth-integrated x-momentum h*u (m^2/s)
  []
  [hv]  # Depth-integrated y-momentum h*v (m^2/s)
  []
[]

[Functions]
  [flat]
    type = ConstantFunction
    value = 0.0
  []
  [h_init]
    type = ParsedFunction
    # value = "if(x<0.5, hL, hR)"
    expression = 'tanh((x-0.5)*20)*0.4+0.6'
    # expression = 1
  []
[]

[UserObjects]
  [flux]
    type = SWENumericalFluxHLL
    gravity = 9.81
    dry_depth = 1e-6
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [outlet]
    type = SWECharacteristicOutflowExactBoundaryFlux
    execute_on = 'INITIAL TIMESTEP_END'
    target_depth = 0.2
    target_un = 0.0
    pressure_weight = 0.0   # start with advective-only; then try small values like 0.1-0.2
  []
  [wall]
    type = SWEWallBoundaryFlux
    execute_on = 'INITIAL TIMESTEP_END'
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
  []
[]

# Aux fields for visualization of water level (eta = h + b)
[AuxVariables]
  [b_field]
    family = MONOMIAL
    order = CONSTANT
  []
  [eta]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  # Cell-constant bathymetry (here zero)
  [b_out]
    type = FunctionAux
    variable = b_field
    function = flat
    execute_on = 'INITIAL TIMESTEP_END'
  []
  # Compute water surface elevation eta = h + b
  [eta_aux]
    type = ParsedAux
    variable = eta
    expression = 'h + b_field'
    coupled_variables = 'h b_field'
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]

[DGKernels]
  [flux_h]
    type = SWEFVFluxDGKernel
    variable = h
    h = h
    hu = hu
    hv = hv
    numerical_flux = flux
    b_var = b_field
  []
  [flux_hu]
    type = SWEFVFluxDGKernel
    variable = hu
    h = h
    hu = hu
    hv = hv
    numerical_flux = flux
    b_var = b_field
  []
  [flux_hv]
    type = SWEFVFluxDGKernel
    variable = hv
    h = h
    hu = hu
    hv = hv
    numerical_flux = flux
    b_var = b_field
  []
  # Hydrostatic correction to preserve eta = const
  [corr_hu]
    type = SWEHydrostaticCorrectionDGKernel
    variable = hu
    h = h
    hu = hu
    hv = hv
    b_var = b_field
  []
  [corr_hv]
    type = SWEHydrostaticCorrectionDGKernel
    variable = hv
    h = h
    hu = hu
    hv = hv
    b_var = b_field
  []
[]

[BCs]
  [obch]
    type = SWEFluxBC
    variable = h
    boundary = 'left'
    h = h
    hu = hu
    hv = hv
    boundary_flux = outlet
  []
  [obchu]
    type = SWEFluxBC
    variable = hu
    boundary = 'left'
    h = h
    hu = hu
    hv = hv
    boundary_flux = outlet
  []
  [obchv]
    type = SWEFluxBC
    variable = hv
    boundary = 'left'
    h = h
    hu = hu
    hv = hv
    boundary_flux = outlet
  []

  [wbch]
    type = SWEFluxBC
    variable = h
    boundary = 'right top bottom'
    h = h
    hu = hu
    hv = hv
    boundary_flux = wall
  []
  [wbchu]
    type = SWEFluxBC
    variable = hu
    boundary = 'right top bottom'
    h = h
    hu = hu
    hv = hv
    boundary_flux = wall
  []
  [wbchv]
    type = SWEFluxBC
    variable = hv
    boundary = 'right top bottom'
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

# [Preconditioning]
#   [fdp]
#     type = FDP
#     full = true
#   []
# []

[Executioner]
  type = Transient
  dt = 1e-2
  num_steps = 100
  nl_abs_tol = 1e-12
  # line_search = NONE
[]

[Outputs]
  exodus = true
  print_linear_residuals = false
[]
