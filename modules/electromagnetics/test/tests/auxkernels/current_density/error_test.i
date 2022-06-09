# This input file is used to test error conditions for the CurrentDensity
# auxkernel. As written, this will fail (missing a coupled variable in that
# auxkernel)

[Mesh]
  [box]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 10
    elem_type = TRI6
  []
[]

[Variables]
  [potential]
    family = LAGRANGE
    order = FIRST
  []
  [electric_field]
    family = NEDELEC_ONE
    order = FIRST
  []
[]

[AuxVariables]
  [J]
    family = NEDELEC_ONE
    order = FIRST
  []
[]

[Kernels]
  [poisson]
    type = Diffusion
    variable = potential
  []
  [EM_curl_curl]
    type = CurlCurlField
    variable = electric_field
  []
[]

[BCs]
# natural BCs for both (all variables = 0)
[]

[AuxKernels]
  [current_density]
    type = ADCurrentDensity
    variable = J
  []
[]

[Materials]
  [conductivity]
    type = ADGenericConstantMaterial
    prop_names = 'electrical_conductivity'
    prop_values = 3.33e2 # electrical conductivity for graphite at 293.15 K
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
[]

[Outputs]
  exodus = true
  print_linear_converged_reason = false
  print_nonlinear_converged_reason = false
[]
