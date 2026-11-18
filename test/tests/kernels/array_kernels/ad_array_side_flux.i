# AD gradients of a coupled array variable on boundary faces. The solution of the diffusion problem
# is u = x (1, 2) on the unit square, so the integrals of grad(u) . n are (1, 2) on the right
# boundary and (-1, -2) on the left boundary.
[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 4
  ny = 4
[]

[Variables]
  [u]
    components = 2
  []
[]

[Kernels]
  [diff_u]
    type = ADArrayDiffusion
    variable = u
  []
[]

[BCs]
  [left]
    type = ADArrayDirichletBC
    variable = u
    boundary = left
    values = '0 0'
  []
  [right]
    type = ADArrayDirichletBC
    variable = u
    boundary = right
    values = '1 2'
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
[]

[Postprocessors]
  [right0]
    type = ADArraySideFluxIntegral
    v = u
    component = 0
    boundary = right
  []
  [right1]
    type = ADArraySideFluxIntegral
    v = u
    component = 1
    boundary = right
  []
  [left0]
    type = ADArraySideFluxIntegral
    v = u
    component = 0
    boundary = left
  []
  [left1]
    type = ADArraySideFluxIntegral
    v = u
    component = 1
    boundary = left
  []
[]

[Outputs]
  csv = true
[]
