# The primary block covers only part of the secondary surface, and the edges of its contact face are
# rounded with radius rf. The minimum_projection_angle check rejects the tilted centre sub-element of
# each primary corner face, which leaves one mortar segment (7 quadrature points for 8 dual shape
# functions) on each secondary corner face.
rf = 0.1
w = 0.3
[Mesh]
  [secondary_block]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 4
    ny = 4
    nz = 1
    xmin = -0.5
    xmax = 0.5
    ymin = -0.5
    ymax = 0.5
    zmin = -0.25
    zmax = 0
    elem_type = HEX20
    boundary_name_prefix = s
  []
  [secondary_id]
    type = SubdomainIDGenerator
    input = secondary_block
    subdomain_id = 1
  []
  [primary_block]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 6
    ny = 6
    nz = 1
    xmin = -${w}
    xmax = ${w}
    ymin = -${w}
    ymax = ${w}
    zmin = 0.005
    zmax = 0.3
    elem_type = HEX20
    boundary_name_prefix = p
    boundary_id_offset = 10
  []
  [fillet]
    type = ParsedNodeTransformGenerator
    input = primary_block
    x_function = 'x'
    y_function = 'y'
    z_function = 'dx := max(abs(x) - (${w} - ${rf}), 0); dy := max(abs(y) - (${w} - ${rf}), 0); '
                 'r := min(sqrt(dx^2 + dy^2), ${rf}); lift := ${rf} - sqrt(${rf}^2 - r^2); '
                 'z + lift * (0.3 - z) / (0.3 - 0.005)'
  []
  [primary_id]
    type = SubdomainIDGenerator
    input = fillet
    subdomain_id = 2
  []
  [combined]
    type = CombinerGenerator
    inputs = 'secondary_id primary_id'
  []
  [secondary_lower]
    type = LowerDBlockFromSidesetGenerator
    input = combined
    sidesets = 's_front'
    new_block_id = 11
    new_block_name = secondary_lower
  []
  [primary_lower]
    type = LowerDBlockFromSidesetGenerator
    input = secondary_lower
    sidesets = 'p_back'
    new_block_id = 12
    new_block_name = primary_lower
  []
[]

[Variables]
  [T]
    block = '1 2'
    order = SECOND
  []
  [lambda]
    block = secondary_lower
    order = SECOND
    use_dual = true
  []
[]

[Kernels]
  [conduction]
    type = Diffusion
    variable = T
    block = '1 2'
  []
[]

[BCs]
  [bottom]
    type = DirichletBC
    variable = T
    boundary = s_back
    value = 0
  []
  [top]
    type = DirichletBC
    variable = T
    boundary = p_front
    value = 1
  []
[]

[Constraints]
  [mortar]
    type = EqualValueConstraint
    primary_boundary = p_back
    secondary_boundary = s_front
    primary_subdomain = primary_lower
    secondary_subdomain = secondary_lower
    variable = lambda
    secondary_variable = T
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
[]

[Postprocessors]
  [T_interface]
    type = PointValue
    variable = T
    point = '0 0 0'
  []
  [lambda_max]
    type = ElementExtremeValue
    variable = lambda
    block = secondary_lower
  []
[]

[Outputs]
  csv = true
[]
