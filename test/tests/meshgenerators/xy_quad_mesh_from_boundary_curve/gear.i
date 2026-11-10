# A gear profile meshed with quadrilaterals in one step. The outer boundary is a sinusoidal
# gear-tooth curve, and the bore is the same profile scaled down and rotated slightly against
# the outer teeth, so the two boundaries exercise the snap independently.
[Mesh]
  [outer_curve]
    type = ParsedCurveGenerator
    x_formula = '(r+a*sin(n*t))*cos(t)'
    y_formula = '(r+a*sin(n*t))*sin(t)'
    section_bounding_t_values = '${fparse 0.0} ${fparse pi} ${fparse 2.0*pi}'
    constant_names = 'r a n'
    constant_expressions = '1.0 0.07 10.0'
    nums_segments = '60 60'
    is_closed_loop = true
  []
  [inner_curve]
    type = ParsedCurveGenerator
    x_formula = '(r+a*sin(n*(t-phi)))*cos(t)'
    y_formula = '(r+a*sin(n*(t-phi)))*sin(t)'
    section_bounding_t_values = '${fparse 0.0} ${fparse pi} ${fparse 2.0*pi}'
    constant_names = 'r a n phi'
    constant_expressions = '0.45 0.035 10.0 0.06'
    nums_segments = '40 40'
    is_closed_loop = true
  []
  [gear]
    type = XYQuadrilateralMeshFromBoundaryCurve
    boundary = 'outer_curve'
    holes = 'inner_curve'
    desired_area = 0.004
    output_boundary = 'rim'
    hole_boundaries = 'bore'
    output_subdomain_name = 'gear'
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

[Postprocessors]
  [area]
    type = VolumePostprocessor
    outputs = csv
  []
[]

[Outputs]
  csv = true
  execute_on = 'FINAL'
[]
