N = 50

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = ${N}
  xmax = 1.0
[]

[GlobalParams]
  family = MONOMIAL
  order = CONSTANT
  gravity = 9.81
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
    expression = "if(x<0.5, hL, hR)"
    symbol_names = 'hL hR'
    symbol_values = 'hL hR'
  []
[]

[UserObjects]
  [flux]
    type = SWENumericalFluxHLL
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
  []
[]

[AuxVariables]
  [b_field]
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
    boundary = 'left right'
    h = h
    hu = hu
    hv = hv
    boundary_flux = wall
  []
  [bchu]
    type = SWEFluxBC
    variable = hu
    boundary = 'left right'
    h = h
    hu = hu
    hv = hv
    boundary_flux = wall
  []
  [bchv]
    type = SWEFluxBC
    variable = hv
    boundary = 'left right'
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
  [h]
    type = LineValueSampler
    end_point = '${fparse 1-0.5/N} 0 0'
    num_points = ${N}
    sort_by = x
    start_point = '${fparse 0.5/N} 0 0'
    variable = h
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]

[Executioner]
  type = Transient

  dt = 1e-3
  nl_abs_tol = 1e-12
  num_steps = 50
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
[]

[Outputs]
  print_linear_residuals = false
  csv = true
  execute_on = 'FINAL'
  checkpoint = false
[]

