# This example couples the product of the curl of a variable vector
# and a supplied field into a diffusion problem of a Lagrange variable
# using automatic differentiation


[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 5
  ny = 5
  nz = 5
  xmin = -1
  ymin = -1
  zmin = -1
  elem_type = HEX20
[]

[Variables]
  [u]
  []
  [v]
    family = NEDELEC_ONE
    order = FIRST
  []
[]

[Kernels]
  [coupled_curl_supplied_field_product]
    type = ADCoupledCurlSuppliedFieldProduct
    variable = u
    vector = v
    supplied_field_x = '0'
    supplied_field_y = '0'
    supplied_field_z = '1'
  []
  [u_diff]
    type = Diffusion
    variable = u
  []
  [u_body_force]
    type = BodyForce
    variable = u
    function = u_body_force_ffn
  []

  [diff]
    type = ADVectorFEWave
    variable = v
    x_forcing_func = x_ffn
    y_forcing_func = y_ffn
  []
[]

[BCs]
  [v_bnd]
    type = VectorCurlPenaltyDirichletBC
    boundary = 'left right top bottom front back'
    penalty = 1e10
    function = v_sln
    variable = v
  []
  [u_bnd]
    type = ADFunctionPenaltyDirichletBC
    boundary = 'left right top bottom front back'
    penalty = 1e10
    function = u_sln
    variable = u
  []
[]

[Functions]
  [x_ffn]
    type = ParsedFunction
    expression = '(2*pi*pi + 1)*cos(pi*x)*sin(pi*y)'
  []
  [y_ffn]
    type = ParsedFunction
    expression = '-(2*pi*pi + 1)*sin(pi*x)*cos(pi*y)'
  []
  [u_body_force_ffn]
    type = ParsedFunction
    expression = '-2*pi*cos(pi*x)*cos(pi*y) + pi^2*cos(z*pi)'
  []

  [u_sln]
    type = ParsedFunction
    expression = 'cos(z*pi)'
  []
  [v_sln]
    type = ParsedVectorFunction
    expression_x = 'cos(pi*x)*sin(pi*y)'
    expression_y = '-sin(pi*x)*cos(pi*y)'
    curl_z = '-2*pi*cos(pi*x)*cos(pi*y)'
  []
[]

[Postprocessors]
  [v_L2Error]
    type = ElementVectorL2Error
    variable = v
    function = v_sln
  []
  [HCurlSemiError]
    type = ElementHCurlSemiError
    variable = v
    function = v_sln
  []
  [HCurlError]
    type = ElementHCurlError
    variable = v
    function = v_sln
  []
  [u_L2Error]
    type = ElementL2Error
    variable = u
    function = u_sln
  []
[]

[Preconditioning]
  [pre]
    type = SMP
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  petsc_options_iname = -pc_type
  petsc_options_value = lu
[]

[Outputs]
  exodus = true
  csv = true
[]
