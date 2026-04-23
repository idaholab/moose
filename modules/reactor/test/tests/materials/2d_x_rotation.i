[Mesh]
  [2d_circle]
    type = ConcentricCircleMeshGenerator
    num_sectors = 8
    radii = '2 3 5'
    rings = '2 1 2'
    has_outer_square = false
    preserve_volumes = true
  []
  [2d_circle2]
    type = ConcentricCircleMeshGenerator
    num_sectors = 6
    radii = '2 3 5'
    rings = '2 1 2'
    has_outer_square = false
    preserve_volumes = true
  []
  [combine]
    type = CombinerGenerator
    inputs = '2d_circle 2d_circle2'
    positions = '0 0 0 11 0 0'
  []
  [extrude]
    type = AdvancedExtruderGenerator
    input = combine
    direction = '0 0 1'
    heights = 2
    num_layers = 2
  []
  [rotate]
    type = TransformGenerator
    input = extrude
    transform = ROTATE_WITH_MATRIX
    rotation_matrix = '0 0 1
                       1 0 0
                       0 1 0'
  []
[]

[Problem]
  kernel_coverage_check = false
  material_coverage_check = false
  solve = false
[]

[AuxVariables]
  [prop]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [set_prop]
    type = MaterialRealAux
    block = '2 3'
    variable = prop
    property = k
    execute_on = timestep_end
  []
[]

[Materials]
  [segment_properties]
    type = GenericConstantMaterial
    block = '2 3'
    prop_names = 'seg1 seg2 seg3 seg4'
    prop_values = '100 200 300 400'
  []
  [rotate_mat]
    type = ControlDrumMaterial
    block = '2 3'
    drum_material_property = k
    rotation_centers = '0 0 0 0 11 0'
    rotation_axis = x
    rotation_angle_functors = '11.25 30'
    segment_angles = '45 90 180 45'
    segment_material_properties = 'seg1 seg2 seg3 seg4'
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
