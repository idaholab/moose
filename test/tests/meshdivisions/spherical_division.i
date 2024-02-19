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
  active = 'spherical_div'
  [spherical_div]
    type = SphericalGridDivision
    center = '1 1 0'

    # Number of bins
    n_radial = 3

    # Extent of the sphere
    r_min = 0.5
    r_max = 4
  []
  [spherical_div_positions]
    type = SphericalGridDivision
    center_positions = center

    # Number of bins
    n_radial = 3

    # Extent of the sphere
    r_min = 0.5
    r_max = 4
  []
[]

[Positions]
  [center]
    type = InputPositions
    positions = '1 1 0'
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
    mesh_division = 'spherical_div'
  []
[]

[VectorPostprocessors]
  [div_out]
    type = ElementValueSampler
    variable = 'div'
    sort_by = 'id'
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
