pin_hex_size = 5
n_rings = 2
hex_size = '${fparse (2*n_rings - 1) * pin_hex_size}'

[Mesh]
  [hex_pin]
    type = PolygonConcentricCircleMeshGenerator
    num_sides = 6
    num_sectors_per_side = '2 2 2 2 2 2'
    # Two position rings
    ring_radii = '${fparse pin_hex_size / 2}'
    ring_intervals = 1
    polygon_size = ${pin_hex_size}
    preserve_volumes = on
  []
  [pattern_assembly]
    type = PatternedHexMeshGenerator
    inputs = 'hex_pin'
    pattern = '0 0;
              0 0 0;
               0 0'
    hexagon_size = ${hex_size}
    background_intervals = 1
    # If we deform the pin mesh to ease the transition with the background,
    # we can get the wrong index for the hexagonal division
    deform_non_circular_region = false
  []
  [pattern_core]
    type = PatternedHexMeshGenerator
    inputs = 'pattern_assembly'
    pattern = '0 0;
              0 0 0;
               0 0'
    generate_core_metadata = true
    pattern_boundary = none
    # If we deform the pin mesh to ease the transition with the background,
    # we can get the wrong index for the hexagonal division
    deform_non_circular_region = false
  []

  # To keep VPP output consistently ordered
  allow_renumbering = false
[]

[Positions]
  [assembly_centers]
    type = InputPositions
    # Assembly centers
    positions = '0 0 0
                 ${fparse -tan(pi/3) * hex_size} ${fparse -hex_size} 0
                 ${fparse -tan(pi/3) * hex_size} ${fparse hex_size} 0
                 0 ${fparse -2 * hex_size} 0
                 0 ${fparse 2 * hex_size} 0
                 ${fparse tan(pi/3) * hex_size} ${fparse -hex_size} 0
                 ${fparse tan(pi/3) * hex_size} ${fparse hex_size} 0'
  []
[]

[MeshDivisions]
  [hexagonal_div]
    type = HexagonalGridDivision
    nr = ${n_rings}
    nz = 1
    lattice_flat_to_flat = '${fparse 2 * hex_size}'
    pin_pitch = '${fparse 2 * pin_hex_size}'
    z_min = 0
    z_max = 0
    center_positions = assembly_centers
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

