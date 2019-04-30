[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 2
  ny = 2
  nz = 2
[]

[Problem]
  solve = false
  kernel_coverage_check = false
[]

[Variables/A]
  family = LAGRANGE_VEC
[]

[ICs/A]
    type = VectorFunctionIC
    variable = A
    function = func
[]

[Functions/func]
  type = ParsedVectorFunction
  value_x = '2*x'
  value_y = '3*y'
  value_z = 'z*z'
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
