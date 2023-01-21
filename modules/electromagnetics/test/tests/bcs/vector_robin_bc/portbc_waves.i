[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 10
    xmin = -1
    ymin = -1
    elem_type = QUAD9
  []
  uniform_refine = 1
[]

[Functions]
  [mms_real] # Manufactured solution, real component
    type = ParsedVectorFunction
    expression_x = 'cos(pi*x)*sin(pi*y)'
    expression_y = '-cos(pi*x)*sin(pi*y)'
    curl_z = 'pi*sin(pi*x)*sin(pi*y) - pi*cos(pi*x)*cos(pi*y)'
  []
  [mms_imaginary] # Manufactured solution, imaginary component
    type = ParsedVectorFunction
    expression_x = 'cos(pi*x + pi/2)*sin(pi*y)'
    expression_y = '-cos(pi*x + pi/2)*sin(pi*x)'
    curl_z = 'pi*sin(pi*x)*cos(pi*y) + pi*sin(pi*y)*cos(pi*x)'
  []
[]

[Variables]
  [u_real]
    family = NEDELEC_ONE
    order = FIRST
  []
  [u_imaginary]
    family = NEDELEC_ONE
    order = FIRST
  []
[]

[Kernels]
  [curl_curl_real]
    type = CurlCurlField
    variable = u_real
  []
  [coeff_real]
    type = VectorFunctionReaction
    variable = u_real
  []
  [rhs_real]
    type = VectorBodyForce
    variable = u_real
    function_x = 'pi*pi*sin(pi*x)*cos(pi*y) + sin(pi*y)*cos(pi*x) + pi*pi*sin(pi*y)*cos(pi*x)'
    function_y = '-pi*pi*sin(pi*x)*cos(pi*y) - pi*pi*sin(pi*y)*cos(pi*x) - sin(pi*y)*cos(pi*x)'
  []
  [curl_curl_imaginary]
    type = CurlCurlField
    variable = u_imaginary
  []
  [coeff_imaginary]
    type = VectorFunctionReaction
    variable = u_imaginary
  []
  [rhs_imaginary]
    type = VectorBodyForce
    variable = u_imaginary
    function_x = '-pi*pi*sin(pi*x)*sin(pi*y) - sin(pi*x)*sin(pi*y) + pi*pi*cos(pi*x)*cos(pi*y)'
    function_y = 'sin(pi*x)*sin(pi*y) + pi*pi*sin(pi*x)*sin(pi*y) - pi*pi*cos(pi*x)*cos(pi*y)'
  []
[]

[BCs]
  [sides_real]
    type = VectorEMRobinBC
    variable = u_real
    component = real
    coupled_field = u_imaginary
    imag_incoming = mms_imaginary
    real_incoming = mms_real
    boundary = 'left right top bottom'
  []
  [sides_imaginary]
    type = VectorEMRobinBC
    variable = u_imaginary
    component = imaginary
    coupled_field = u_real
    imag_incoming = mms_imaginary
    real_incoming = mms_real
    boundary = 'left right top bottom'
  []
[]

[Preconditioning]
  [SMP]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
[]

[Outputs]
  exodus = true
[]
