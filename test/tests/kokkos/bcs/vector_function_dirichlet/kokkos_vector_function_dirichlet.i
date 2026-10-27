# Vector diffusion on the unit square with u = (1 + x*y, x - y) imposed on the boundary through
# component functions. The exact solution is harmonic and bilinear in each component, so it is
# reproduced on QUAD4 elements.

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 4
  ny = 4
[]

[Variables]
  [u]
    family = LAGRANGE_VEC
  []
[]

[Functions]
  [exact_x]
    type = KokkosParsedFunction
    expression = '1 + x*y'
  []
  [exact_y]
    type = KokkosParsedFunction
    expression = 'x - y'
  []
[]

[Kernels]
  [diff]
    type = KokkosVectorDiffusion
    variable = u
  []
[]

[BCs]
  [left_right]
    type = KokkosVectorFunctionDirichletBC
    variable = u
    boundary = 'left right'
    function_x = exact_x
    function_y = exact_y
  []
  [top_bottom]
    type = KokkosVectorFunctionDirichletBC
    variable = u
    boundary = 'top bottom'
    function_x = exact_x
    function_y = exact_y
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
[]

[Outputs]
  exodus = true
[]
