# A rigid line of EDGE2 elements without an interior parent is pushed into the top of an elastic
# block. Its normal from the node ordering, -y, points toward the block as node-face contact
# requires. With frictionless contact, free lateral faces and zero Poisson's ratio, the block is
# in uniaxial stress, so the contact pressure is E * delta / H.

E = 1e3
H = 1
delta = 1e-2

[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Mesh]
  [block]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 3
    ny = 2
    ymax = ${H}
  []
  [line]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 4
    xmin = -0.2
    xmax = 1.2
    subdomain_ids = 1
    boundary_name_prefix = surface
    boundary_id_offset = 10
  []
  [lift]
    type = TransformGenerator
    input = line
    transform = TRANSLATE
    vector_value = '0 ${H} 0'
  []
  [combine]
    type = CombinerGenerator
    inputs = 'block lift'
  []
  [surface_name]
    type = RenameBlockGenerator
    input = combine
    old_block = 1
    new_block = surface
  []
  [primary]
    type = ParsedGenerateNodeset
    input = surface_name
    new_nodeset_name = rigid_surface
    expression = '1'
    included_subdomains = surface
    allow_distributed_meshes = true
  []
[]

[Physics/SolidMechanics/QuasiStatic]
  [block]
    block = 0
    strain = SMALL
  []
[]

# The rigid surface carries displacements, prescribed by Dirichlet conditions, so that it moves
# with the displaced mesh
[Variables]
  [disp_x]
  []
  [disp_y]
  []
[]

[Functions]
  [push]
    type = ParsedFunction
    expression = '-${delta} * t'
  []
[]

[BCs]
  [bottom_y]
    type = DirichletBC
    variable = disp_y
    boundary = bottom
    value = 0
  []
  [left_x]
    type = DirichletBC
    variable = disp_x
    boundary = left
    value = 0
  []
  [surface_x]
    type = DirichletBC
    variable = disp_x
    boundary = rigid_surface
    value = 0
  []
  [surface_y]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = rigid_surface
    function = push
  []
[]

[Contact]
  [rigid]
    primary = rigid_surface
    secondary = top
    primary_surface_blocks = surface
    formulation = mortar
    model = frictionless
  []
[]

[Materials]
  [elasticity]
    type = ComputeIsotropicElasticityTensor
    block = 0
    youngs_modulus = ${E}
    poissons_ratio = 0
  []
  [stress]
    type = ComputeLinearElasticStress
    block = 0
  []
[]

[Postprocessors]
  [stress_yy]
    type = ElementAverageValue
    variable = stress_yy
    block = 0
  []
  # Lateral expansion of the contact surface, nu / (1 - nu) * delta without friction and zero when
  # friction keeps it stuck to the rigid surface
  [disp_x_top]
    type = PointValue
    variable = disp_x
    point = '1 ${H} 0'
  []
  [analytical_stress_yy]
    type = FunctionValuePostprocessor
    function = '-${E} * ${delta} * t / ${H}'
  []
[]

[AuxVariables]
  [stress_yy]
    order = CONSTANT
    family = MONOMIAL
    block = 0
  []
[]

[AuxKernels]
  [stress_yy]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_yy
    index_i = 1
    index_j = 1
    block = 0
  []
[]

[Problem]
  # The rigid surface has no physics, only prescribed displacements
  kernel_coverage_check = false
  material_coverage_check = false
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  petsc_options_iname = '-pc_type -pc_factor_shift_type'
  petsc_options_value = 'lu       NONZERO'
  dt = 1
  end_time = 1
  nl_abs_tol = 1e-10
[]

[Outputs]
  csv = true
[]
