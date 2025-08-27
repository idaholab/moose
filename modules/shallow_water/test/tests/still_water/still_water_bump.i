# Shallow-water still-water (lake-at-rest) over a smooth bump
#
# Variables and meaning:
# - h  : water depth (m), the conservative height variable
# - hu : depth-integrated x-momentum (m^2/s), equal to h*u
# - hv : depth-integrated y-momentum (m^2/s), equal to h*v
#
# Water surface elevation (free-surface level) is
#   eta = h + b
# where b is the bathymetry (bed elevation). In this input, b is provided
# by the SWEBathymetry Material from the ParsedFunction 'bump' below.
#
# To visualize the water level eta, we create two AuxVariables:
#   - b_field: outputs the material property 'b' to a field via MaterialRealAux
#   - eta    : computes h + b_field via ParsedAux

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 20
  ny = 20
  xmax = 1.0
  ymax = 1.0
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
  [bump]
    type = ParsedFunction
    value = "0.1*exp(-100*((x-0.5)^2 + (y-0.5)^2))"
  []
  [eta]
    type = ConstantFunction
    value = 1.0
  []
  [h_init]
    type = ParsedFunction
    value = "max(eta-bump, 0)"
    vars = 'eta bump'
    vals = 'eta bump'
  []
[]

[UserObjects]
  [flux]
    type = SWENumericalFluxHLL
    gravity = 9.81
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
[]

# Aux fields for visualization of water level
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
  # Export bathymetry material property 'b' to a field
  [b_out]
    type = FunctionAux
    variable = b_field
    function = bump
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
  dt = 1e-2
  num_steps = 150
  nl_abs_tol = 1e-12
[]

[Outputs]
  exodus = true
  print_linear_residuals = false
[]
