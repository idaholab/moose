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
  active = 'cartesian_div'
  [cartesian_div]
    type = CartesianGridDivision
    bottom_left = '0.1 0.5 0.5'
    top_right = '5 2 1'
    nx = '3'
    ny = '4'
    nz = '1'
  []
  [cartesian_div_center]
    type = CartesianGridDivision
    center = '2.55 1.25 0.75'
    widths = '4.9 1.5 0.5'
    nx = '3'
    ny = '4'
    nz = '1'
  []
  [cartesian_div_center_pos]
    type = CartesianGridDivision
    center_positions = 'center'
    widths = '4.9 1.5 0.5'
    nx = '3'
    ny = '4'
    nz = '1'
  []
[]

[Positions]
  [center]
    type = InputPositions
    positions = '2.55 1.25 0.75'
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
    mesh_division = 'cartesian_div'
  []
[]

[VectorPostprocessors]
  [div_out]
    type = ElementValueSampler
    variable = 'div'
    sort_by = 'id'
    outputs = csv
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

