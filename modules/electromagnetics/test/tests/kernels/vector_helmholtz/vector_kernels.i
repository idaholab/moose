# Test for EM module vector kernels CurlCurlField and VectorFunctionReaction
# Manufactured solution: u = y * x_hat - x * y_hat

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
[]

[Variables]
  [u]
    family = NEDELEC_ONE
    order = FIRST
  []
[]

[Kernels]
  [curl_curl]
    type = CurlCurlField
    variable = u
  []
  [coeff]
    type = VectorFunctionReaction
    variable = u
  []
  [rhs]
    type = VectorBodyForce
    variable = u
    function_x = 'y'
    function_y = '-x'
  []
[]

[BCs]
  [sides]
    type = VectorCurlPenaltyDirichletBC
    variable = u
    function_x = 'y'
    function_y = '-x'
    penalty = 1e8
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
