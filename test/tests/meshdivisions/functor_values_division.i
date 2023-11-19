[Mesh]
  [cmg]
    type = CartesianMeshGenerator
    dim = 3
    dx = '1.5 2.4 0.1'
    dy = '1.3 0.9'
    dz = '0.4 0.5 0.6 0.7'
    ix = '2 1 1'
    iy = '2 3'
    iz = '1 1 1 1'
  []
  # To keep VPP output consistently ordered
  allow_renumbering = false
[]

[MeshDivisions]
  [functor_div]
    type = FunctorBinnedValuesDivision
    min_value = 0.1
    max_value = 2
    num_bins = 3
    functor = 'xf'
  []
[]

[Functions]
  [xf]
    type = ParsedFunction
    expression = 'x'
  []
[]

[AuxVariables]
  [div]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [mesh_div]
    type = MeshDivisionAux
    variable = div
    mesh_division = 'functor_div'
  []
[]

[VectorPostprocessors]
  [div_out]
    type = ElementValueSampler
    variable = 'div'
    sort_by = 'id'
  []
[]

[Postprocessors]
  [ndiv]
    type = NumMeshDivisions
    mesh_division = functor_div
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[Outputs]
  csv = true
[]
