# A rigid plane of QUAD4 elements without an interior parent is pushed into the bottom of an elastic
# block. Its normal from the node ordering, +z, points toward the block as node-face contact
# requires. With frictionless contact, free lateral faces and zero Poisson's ratio, the block is
# in uniaxial stress, so the contact pressure is E * delta / H.

E = 1e3
H = 1
delta = 1e-2

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Mesh]
  [block]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 2
    ny = 2
    nz = 2
    zmax = ${H}
  []
  [plane]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 2
    ny = 2
    xmin = -0.2
    xmax = 1.2
    ymin = -0.2
    ymax = 1.2
    subdomain_ids = 1
    boundary_name_prefix = surface
    boundary_id_offset = 10
  []
  [combine]
    type = CombinerGenerator
    inputs = 'block plane'
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
  [disp_z]
  []
[]

[Functions]
  [push]
    type = ParsedFunction
    expression = '${delta} * t'
  []
[]

[BCs]
  [front_z]
    type = DirichletBC
    variable = disp_z
    boundary = front
    value = 0
  []
  [left_x]
    type = DirichletBC
    variable = disp_x
    boundary = left
    value = 0
  []
  [bottom_y]
    type = DirichletBC
    variable = disp_y
    boundary = bottom
    value = 0
  []
  [surface_x]
    type = DirichletBC
    variable = disp_x
    boundary = rigid_surface
    value = 0
  []
  [surface_y]
    type = DirichletBC
    variable = disp_y
    boundary = rigid_surface
    value = 0
  []
  [surface_z]
    type = FunctionDirichletBC
    variable = disp_z
    boundary = rigid_surface
    function = push
  []
[]

[Contact]
  [rigid]
    primary = rigid_surface
    secondary = back
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
  [stress_zz]
    type = ElementAverageValue
    variable = stress_zz
    block = 0
  []
  [analytical_stress_zz]
    type = FunctionValuePostprocessor
    function = '-${E} * ${delta} * t / ${H}'
  []
[]

[AuxVariables]
  [stress_zz]
    order = CONSTANT
    family = MONOMIAL
    block = 0
  []
[]

[AuxKernels]
  [stress_zz]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_zz
    index_i = 2
    index_j = 2
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
