# A constant flux is imposed on each component of a vector variable on 'right' while 'left' is held
# at zero, so that vector diffusion on the unit square gives u_x = x and u_y = 2x.

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 4
    ny = 4
  []
[]

[Variables]
  [u]
    family = LAGRANGE_VEC
    order = FIRST
  []
[]

[Kernels]
  [diff]
    type = KokkosVectorDiffusion
    variable = u
  []
[]

[BCs]
  [fixed]
    type = KokkosVectorDirichletBC
    variable = u
    boundary = left
    values = '0 0 0'
  []
  [flux]
    type = KokkosVectorNeumannBC
    variable = u
    boundary = right
    values = '1 2 0'
  []
[]

[AuxVariables]
  [u_x]
    order = FIRST
    family = LAGRANGE
  []
  [u_y]
    order = FIRST
    family = LAGRANGE
  []
[]

[AuxKernels]
  [u_x]
    type = VectorVariableComponentAux
    variable = u_x
    vector_variable = u
    component = x
  []
  [u_y]
    type = VectorVariableComponentAux
    variable = u_y
    vector_variable = u
    component = y
  []
[]

[Postprocessors]
  # On the flux boundary the solution is u_x = 1 and u_y = 2, so these values check the magnitude of
  # the imposed flux as well as its distribution over the components.
  [u_x_right]
    type = PointValue
    variable = u_x
    point = '1 0.5 0'
  []
  [u_y_right]
    type = PointValue
    variable = u_y
    point = '1 0.5 0'
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
[]

[Outputs]
  csv = true
[]
