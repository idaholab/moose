# Every node of a cube is moved by the finite rigid motion u = t + (R - I) (X - X_ref): a
# translation of the reference point X_ref = (0.5, 0.5, 0.5) by (0.3, -0.2, 0.1) t and a rotation
# by 2 pi / 3 t about (1, 1, 1). At t = 1 the rotation maps x -> y -> z -> x, so the exact
# displacement is t + (z, x, y) - (x, y, z), measured from X_ref.

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Mesh]
  [cube]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 2
    ny = 2
    nz = 2
  []
  [all_nodes]
    type = ParsedGenerateNodeset
    input = cube
    new_nodeset_name = all_nodes
    expression = 'x > -1'
  []
[]

[Problem]
  # Every degree of freedom is prescribed
  kernel_coverage_check = false
[]

[Variables]
  [disp_x]
  []
  [disp_y]
  []
  [disp_z]
  []
[]

[Functions]
  [tx]
    type = ParsedFunction
    expression = '0.3 * t'
  []
  [ty]
    type = ParsedFunction
    expression = '-0.2 * t'
  []
  [tz]
    type = ParsedFunction
    expression = '0.1 * t'
  []
  # Each component of the rotation vector 2 pi / 3 (1, 1, 1) / sqrt(3)
  [theta]
    type = ParsedFunction
    expression = '2 * pi / (3 * sqrt(3)) * t'
  []
  [exact_x]
    type = ParsedFunction
    expression = '0.3 + (z - 0.5) - (x - 0.5)'
  []
  [exact_y]
    type = ParsedFunction
    expression = '-0.2 + (x - 0.5) - (y - 0.5)'
  []
  [exact_z]
    type = ParsedFunction
    expression = '0.1 + (y - 0.5) - (z - 0.5)'
  []
[]

[BCs]
  [rigid_x]
    type = RigidBodyDisplacementBC
    variable = disp_x
    component = x
    boundary = all_nodes
    reference_point = '0.5 0.5 0.5'
    translations = 'tx ty tz'
    rotations = 'theta theta theta'
  []
  [rigid_y]
    type = RigidBodyDisplacementBC
    variable = disp_y
    component = y
    boundary = all_nodes
    reference_point = '0.5 0.5 0.5'
    translations = 'tx ty tz'
    rotations = 'theta theta theta'
  []
  [rigid_z]
    type = RigidBodyDisplacementBC
    variable = disp_z
    component = z
    boundary = all_nodes
    reference_point = '0.5 0.5 0.5'
    translations = 'tx ty tz'
    rotations = 'theta theta theta'
  []
[]

[Postprocessors]
  [error_x]
    type = NodalL2Error
    variable = disp_x
    function = exact_x
  []
  [error_y]
    type = NodalL2Error
    variable = disp_y
    function = exact_y
  []
  [error_z]
    type = NodalL2Error
    variable = disp_z
    function = exact_z
  []
[]

[Executioner]
  # One step to t = 1, where the exact solution above holds
  type = Transient
  num_steps = 1
[]

[Outputs]
  csv = true
[]
