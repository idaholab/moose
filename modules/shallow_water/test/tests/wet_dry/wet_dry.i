# Agitated lake with initially dry shore

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 15
  ny = 15
  xmax = 1.0
  ymax = 1.0
[]

[GlobalParams]
  family = MONOMIAL
  order = CONSTANT
  gravity = 9.81
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
  [lakebed]
    type = ParsedFunction
    value = "1.0 - exp(-100*(((x-0.5)/4)^2 + ((y-0.5)/4)^2))"
  []
  [eta]
    # asymmetric surface perturbation
    type = ParsedFunction
    value = "0.5 + 0.5*exp(-100*((x-0.5)^2 + ((y-0.5)/3)^2))"
  []
  [h_init]
    type = ParsedFunction
    expression = "max(eta-lakebed, 0)"
    symbol_names  = 'eta lakebed'
    symbol_values = 'eta lakebed'
  []
[]

[UserObjects]
  [flux]
    type = SWENumericalFluxHLL
    dry_depth = 1e-6
    execute_on = 'INITIAL TIMESTEP_END'
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

# Aux fields for visualization of water level
[AuxVariables]
  [b_field]
  []
  [eta]
  []
[]

[AuxKernels]
  # Export bathymetry material property 'b' to a field
  [b_out]
    type = FunctionAux
    variable = b_field
    function = lakebed
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
  # Note: We omit explicit bed-slope sources here because the numerical flux
  # uses hydrostatic reconstruction with bathymetry, which is well-balanced for
  # the lake-at-rest state. Adding a separate source term would double-count
  # topographic effects and destroy well-balancing.
[]

[Executioner]
  type = Transient
  dt = 2e-2
  num_steps = 10
  nl_abs_tol = 1e-12
[]

[Outputs]
  exodus = true
  print_linear_residuals = false
  hide = 'eta hu hv b_field'
[]
