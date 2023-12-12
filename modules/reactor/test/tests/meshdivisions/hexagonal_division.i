# Increase resolution to check results visually
resolution = 3
z_resolution = 3

[Mesh]
  [cmg]
    type = CartesianMeshGenerator
    dim = 3
    dx = '1.5 2.4 0.1'
    dy = '1.3 0.9'
    dz = '0.4 0.5 0.6 0.7'
    ix = '${fparse 2 * resolution} ${fparse resolution} ${fparse z_resolution}'
    iy = '${fparse 2 * resolution} ${fparse 3 * z_resolution}'
    iz = '${fparse resolution} ${fparse resolution} ${fparse resolution} ${fparse z_resolution}'
  []
  # To keep VPP output consistently ordered
  allow_renumbering = false
[]

[MeshDivisions]
  [hexagonal_div]
    type = HexagonalGridDivision
    nr = 4
    nz = 2
    lattice_flat_to_flat = 4
    pin_pitch = 0.6
    z_min = 0
    z_max = 3
    center = '1 1 0'
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
    mesh_division = 'hexagonal_div'
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

[Postprocessors]
  [npos]
    type = NumMeshDivisions
    mesh_division = 'hexagonal_div'
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

