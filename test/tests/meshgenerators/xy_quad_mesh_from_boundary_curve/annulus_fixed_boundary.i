# An annulus meshed with quadrilaterals whose boundary nodes are the nodes of the input curves:
# the triangulation is not allowed to split the boundary segments, even though the desired area
# asks for elements smaller than those segments, so the mesh can be stitched to a neighbor that
# shares the curves. The recombination keeps the triangles it cannot merge, because the
# elimination of the leftover triangles would add a node in the middle of every boundary segment.
[Mesh]
  [outer_curve]
    type = ParsedCurveGenerator
    x_formula = 'r*cos(t)'
    y_formula = 'r*sin(t)'
    section_bounding_t_values = '${fparse 0.0} ${fparse pi} ${fparse 2.0*pi}'
    constant_names = 'r'
    constant_expressions = '1.0'
    nums_segments = '16 16'
    is_closed_loop = true
  []
  [inner_curve]
    type = ParsedCurveGenerator
    x_formula = 'r*cos(t)'
    y_formula = 'r*sin(t)'
    section_bounding_t_values = '${fparse 0.0} ${fparse pi} ${fparse 2.0*pi}'
    constant_names = 'r'
    constant_expressions = '0.35'
    nums_segments = '6 6'
    is_closed_loop = true
  []
  [annulus]
    type = XYQuadrilateralMeshFromBoundaryCurve
    boundary = 'outer_curve'
    holes = 'inner_curve'
    refine_boundary = false
    refine_holes = 'false'
    desired_area = 0.005
    all_quad = false
    output_boundary = 'rim'
    hole_boundaries = 'bore'
    output_subdomain_name = 'annulus'
    parsed_curve_generators = 'outer_curve inner_curve'
    snap_boundaries = 'rim bore'
  []
  # The recombination breaks score ties by element id, so keep the ids stable
  allow_renumbering = false
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Transient
  num_steps = 1
[]

[AuxVariables]
  [one]
    initial_condition = 1
  []
[]

[Postprocessors]
  [area]
    type = VolumePostprocessor
    outputs = csv
  []
  # The node counts of the boundaries are those of the input curves when no segment was split
  [rim_nodes]
    type = NodalSum
    variable = one
    boundary = 'rim'
    outputs = csv
  []
  [bore_nodes]
    type = NodalSum
    variable = one
    boundary = 'bore'
    outputs = csv
  []
[]

[Outputs]
  csv = true
  execute_on = 'FINAL'
[]
