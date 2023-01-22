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
    function_x = func
[]

[Functions/func]
  type = ParsedFunction
  expression = '2*x'
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
