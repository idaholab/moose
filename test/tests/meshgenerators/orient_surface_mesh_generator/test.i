[Mesh]
  [2d_surface]
    type = CartesianMeshGenerator
    dim = 2
    dx = 10
    dy = 1
    ix = 2
    iy = 2
  []
  [flip_side]
    type = OrientSurfaceMeshGenerator
    input = '2d_surface'
    normal_to_align_with = '0 0 -1'
  []
[]


[Executioner]
  type = Steady
[]

[Problem]
  solve = false
[]

[Outputs]
  [out]
    type = Exodus
    execute_on = FINAL
  []
[]

[AuxVariables]
  [n_x]
    family = MONOMIAL
    order = CONSTANT
    [AuxKernel]
      type = ElementNormalAux
      component = x
    []
  []
  [n_y]
    family = MONOMIAL
    order = CONSTANT
    [AuxKernel]
      type = ElementNormalAux
      component = y
    []
  []
  [n_z]
    family = MONOMIAL
    order = CONSTANT
    [AuxKernel]
      type = ElementNormalAux
      component = z
    []
  []
[]
