# Diffusion on the unit square with u = 1 + x*y imposed on all sides through a function. The exact
# solution is u = 1 + x*y, which is harmonic and bilinear, so it is reproduced on QUAD4 elements.

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 4
  ny = 4
[]

[Variables]
  [u]
  []
[]

[Functions]
  [exact]
    type = KokkosParsedFunction
    expression = '1 + x*y'
  []
[]

[Kernels]
  [diff]
    type = KokkosDiffusion
    variable = u
  []
[]

[BCs]
  [all]
    type = KokkosFunctionDirichletBC
    variable = u
    boundary = 'left right top bottom'
    function = exact
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
[]

[Outputs]
  exodus = true
[]
